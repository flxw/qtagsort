# include <QNetworkRequest>
# include <QNetworkReply>
# include <QUrl>
# include <QScriptValueIterator>

# include "fingerprinter.h"
# include "versioninfo.h"


extern "C" {
# define __STDC_CONSTANT_MACROS
# include <libavcodec/avcodec.h>
# include <libavformat/avformat.h>
# include <libavutil/opt.h>
# include <libswresample/swresample.h>
# ifdef _WIN32
# include <windows.h>
# endif
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BUFFER_SIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 94, 1)
#define AV_SAMPLE_FMT_S16 SAMPLE_FMT_S16
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#endif

/* methods =================================== */
Fingerprinter::Fingerprinter(QObject *parent) : QObject(parent) {
    this->nwaManager = new QNetworkAccessManager(this);

    connect(this->nwaManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(receiveNetAnswer(QNetworkReply *)));
    connect(this->nwaManager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)),
            this, SLOT(networkAccessChanged(QNetworkAccessManager::NetworkAccessibility)));
}


void Fingerprinter::getMusicBrainzData(const QString &filename) {
    emit status(tr("Building acoustID request..."), 5000);

    QString request("http://api.acoustid.org/v2/lookup");
    int duration;

    request.append("?client=ObrsWKGE");
    request.append("&meta=recordings+releasegroups+compress");
    request.append("&fingerprint=").append(getFingerprint(filename, duration));
    request.append("&duration=").append(QString::number(duration));

    this->nwaManager->get(QNetworkRequest(QUrl(request)));

    emit status(tr("Built acoustID request....querying server..."), 5000);
}

QString Fingerprinter::getFingerprint(const QString &filename, int &duration) {
    QString qfingerprint;
    QByteArray filenameByteArray = filename.toLocal8Bit();
    ChromaprintContext *chromaprint_ctx;
    char *file_name, *fingerprint;
    int algo = CHROMAPRINT_ALGORITHM_DEFAULT;
    int max_length = 120;

    /* initialize the av* libraries */
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);

    /* allocate fingerprinter context */
    chromaprint_ctx = chromaprint_new(algo);
    file_name       = filenameByteArray.data();

    if (!decode_audio_file(chromaprint_ctx, file_name, max_length, &duration)) {
        qDebug("ERROR: unable to calculate fingerprint for file %s", file_name);
    }

    if (!chromaprint_get_fingerprint(chromaprint_ctx, &fingerprint)) {
        qDebug("ERROR: unable to calculate fingerprint for file %s, skipping", file_name);
    }

    /* save fingerprint */
    qfingerprint = QString::fromLocal8Bit(fingerprint);

    /* free contexts and buffers */
    chromaprint_dealloc(fingerprint);
    chromaprint_free(chromaprint_ctx);

    return qfingerprint;
}

int Fingerprinter::decode_audio_file(ChromaprintContext *chromaprint_ctx, const char *file_name, int max_length, int *duration) {
    int ok = 0, codec_ctx_opened = 0, remaining, length, consumed, got_frame, stream_index;
    int max_dst_nb_samples = 0, dst_linsize = 0;
    uint8_t *dst_data[1] = { NULL };
    uint8_t **data;
    AVFormatContext *format_ctx = NULL;
    AVCodecContext *codec_ctx   = NULL;
    AVCodec *codec               = NULL;
    AVStream *stream             = NULL;
    AVFrame *frame               = NULL;
    AVPacket packet;
    SwrContext *swr_ctx = NULL;

    if (avformat_open_input(&format_ctx, file_name, NULL, NULL) != 0) {
        qDebug("ERROR: couldn't open the file\n");
        goto done;
    }

    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        qDebug("ERROR: couldn't find stream information in the file\n");
        goto done;
    }

    stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (stream_index < 0) {
        qDebug("ERROR: couldn't find any audio stream in the file\n");
        goto done;
    }

    stream = format_ctx->streams[stream_index];

    codec_ctx = stream->codec;
    codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        qDebug("ERROR: couldn't open the codec\n");
        goto done;
    }

    codec_ctx_opened = 1;

    if (codec_ctx->channels <= 0) {
        qDebug("ERROR: no channels found in the audio stream\n");
        goto done;
    }

    if (codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
        swr_ctx = swr_alloc_set_opts(NULL,
            codec_ctx->channel_layout, AV_SAMPLE_FMT_S16, codec_ctx->channel_layout,
            codec_ctx->channel_layout, codec_ctx->sample_fmt, codec_ctx->channel_layout,
            0, NULL);

        if (!swr_ctx) {
            qDebug("ERROR: couldn't allocate audio converter\n");
            goto done;
        }

        if (swr_init(swr_ctx) < 0) {
            qDebug("ERROR: couldn't initialize the audio converter\n");
            goto done;
        }
    }

    *duration = stream->time_base.num * stream->duration / stream->time_base.den;
    remaining = max_length * codec_ctx->channels * codec_ctx->sample_rate;

    chromaprint_start(chromaprint_ctx, codec_ctx->sample_rate, codec_ctx->channels);

    frame = avcodec_alloc_frame();

    while (1) {
        if (av_read_frame(format_ctx, &packet) < 0) {
            break;
        }

        if (packet.stream_index == stream_index) {
            avcodec_get_frame_defaults(frame);

            got_frame = 0;
            consumed = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);

            if (consumed < 0) {
                qDebug("WARNING: error decoding audio\n");
                continue;
            }

            if (got_frame) {
                data = frame->data;

                if (swr_ctx) {
                    if (frame->nb_samples > max_dst_nb_samples) {
                        av_freep(&dst_data[0]);

                        if (av_samples_alloc(dst_data, &dst_linsize, codec_ctx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0) {
                            qDebug("ERROR: couldn't allocate audio converter buffer\n");
                            goto done;
                        }

                        max_dst_nb_samples = frame->nb_samples;
                    }

                    if (swr_convert(swr_ctx, dst_data, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples) < 0) {
                        qDebug("ERROR: couldn't convert the audio\n");
                        goto done;
                    }

                    data = dst_data;
                }

                length = MIN(remaining, frame->nb_samples * codec_ctx->channels);

                if (!chromaprint_feed(chromaprint_ctx, data[0], length)) {
                    goto done;
                }

                if (max_length) {
                    remaining -= length;
                    if (remaining <= 0) goto finish;
                }
            }
        }

        av_free_packet(&packet);
    }

finish:
    if (!chromaprint_finish(chromaprint_ctx)) {
        qDebug("ERROR: fingerprint calculation failed\n");
        goto done;
    }

    ok = 1;

done:
    if (frame) {
        avcodec_free_frame(&frame);
    }

    if (dst_data[0]) {
        av_freep(&dst_data[0]);
    }

    if (swr_ctx) {
        swr_free(&swr_ctx);
    }

    if (codec_ctx_opened) {
        avcodec_close(codec_ctx);
    }

    if (format_ctx) {
        avformat_close_input(&format_ctx);
    }

    return ok;
}

/* slots ===================================== */
void Fingerprinter::receiveNetAnswer(QNetworkReply *reply) {
    if (QNetworkReply::NoError == reply->error()) {
        QString jsonString = QString(reply->readAll());
        QScriptValue scValue = scriptEngine.evaluate("(" + jsonString + ")");

        /* the result contains an OBJECT in the results list
             * for each matched fingerprint!
             * as a consequence, we need to iterate over these objects
             * an call their properties as appropriate
             *
             * ====== this is the results object structure ========
             "results": [{
                "score": 1.0,
                "id": "9ff43b6a-4f16-427c-93c2-92307ca505e0",
                >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                "recordings": [{
                  "duration": 639,
                  ================================================
                  "releasegroups": [{
                    "type": "Album",
                    "id": "ddaa2d4d-314e-3e7c-b1d0-f6d207f5aa2f",
                    "title": "Before the Dawn Heals Us"
                  }],
                  ================================================
                  "title": "Lower Your Eyelids to Die With the Sun",
                  "id": "cd2e7c47-16f5-46c6-a37c-a1eb7bf599ff",
                  ================================================
                    "artists": [{
                    "id": "6d7b7cd4-254b-4c25-83f6-dd20f98ceacd",
                    "name": "M83"
                  }]
                  ================================================
                }]
                <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              }]
             */
        if (scValue.property("status").toString() == QString("ok")) {
            QScriptValueIterator resultIterator(scValue.property("results"));
            QStringList titleList, artistList, releaseList;
            QString buffer;

            while (resultIterator.hasNext()) {
                resultIterator.next();

                QScriptValueIterator recordingsIterator(resultIterator.value().property("recordings"));

                while (recordingsIterator.hasNext()) {
                    recordingsIterator.next();

                    buffer = recordingsIterator.value().property("title").toString();
                    if (!titleList.contains(buffer) && !buffer.isEmpty()) titleList.append(buffer);

                    QScriptValueIterator artistIterator(recordingsIterator.value().property("artists"));
                    while (artistIterator.hasNext()) {
                        artistIterator.next();
                        buffer = artistIterator.value().property("name").toString();
                        if (!artistList.contains(buffer) && !buffer.isEmpty()) artistList.append(buffer);
                    }

                    QScriptValueIterator releaseIterator(recordingsIterator.value().property("releasegroups"));
                    while (releaseIterator.hasNext()) {
                        releaseIterator.next();
                        buffer = releaseIterator.value().property("title").toString();
                        if (!releaseList.contains(buffer) && !buffer.isEmpty()) releaseList.append(buffer);
                    }
                }
            }

            if (titleList.length() || artistList.length() || releaseList.length()) {
                emit status(tr("Received good answer...presenting matches...."), 5000);
                emit receivedGoodAnswer(titleList, releaseList, artistList);
            }
        } else {
            emit status(tr("Fingerprint could not be matched... :("), 5000);
        }
    } else {
        emit status(tr("Network error - QNetworkReply error code %1").arg(reply->error()), 5000);
    }

    reply->deleteLater();
}

void Fingerprinter::networkAccessChanged(QNetworkAccessManager::NetworkAccessibility na) {
    emit status(tr("Network accessibility changed! Network access now %1!").arg((na > 0)?"accessible":"unavailable"), 5000);
}
