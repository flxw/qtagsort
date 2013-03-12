# include <QNetworkRequest>
# include <QNetworkReply>
# include <QUrl>
# include <QScriptValueIterator>
# include <string.h>

# include "fingerprinter.h"
# include "versioninfo.h"


extern "C" {
# define __STDC_CONSTANT_MACROS
# include <libavcodec/avcodec.h>
# include <libavformat/avformat.h>
# include "ffmpeg/audioconvert.h"
# include "ffmpeg/samplefmt.h"
# include <chromaprint.h>
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

    /* TODO: Get my own API key! */
    request.append("?client=8XaBELgH");
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
    int16_t *buffer1, *buffer2;

    /* initialize the av* libraries */
    av_register_all();
    av_log_set_level(AV_LOG_ERROR);
    /* allocate buffer memory and fingerprinter context */
    buffer1 = (int16_t*)av_malloc(BUFFER_SIZE + 16);
    buffer2 = (int16_t*)av_malloc(BUFFER_SIZE + 16);
    chromaprint_ctx = chromaprint_new(algo);

    file_name = filenameByteArray.data();
    if (!decode_audio_file(chromaprint_ctx, buffer1, buffer2, file_name, max_length, &duration)) {
        qDebug("ERROR: unable to calculate fingerprint for file %s", file_name);
    }

    if (!chromaprint_get_fingerprint(chromaprint_ctx, &fingerprint)) {
        qDebug("ERROR: unable to calculate fingerprint for file %s, skipping", file_name);
    }

    /* save fingerprint */
    qfingerprint = QString::fromLocal8Bit(fingerprint, strlen(fingerprint));

    /* free contexts and buffers */
    chromaprint_dealloc(fingerprint);
    chromaprint_free(chromaprint_ctx);
    av_free(buffer1);
    av_free(buffer2);

    return qfingerprint;
}

int Fingerprinter::decode_audio_file(ChromaprintContext *chromaprint_ctx,
                                     int16_t *buffer1, int16_t *buffer2,
                                     const char *file_name,
                                     int max_length, int *duration) {
    AVFormatContext *format_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    AVCodec *codec = NULL;
    AVStream *stream = NULL;
    AVPacket packet, packet_temp;
    AVAudioConvert *convert_ctx = NULL;
    int16_t *buffer;
    int i, ok = 0, remaining, length, consumed, buffer_size, codec_ctx_opened = 0;

    if (avformat_open_input(&format_ctx, file_name, NULL, NULL) != 0) {
        qDebug("ERROR: couldn't open the file");
        goto done;
    }

    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        qDebug("ERROR: couldn't find stream information in the file");
        goto done;
    }

    for (i = 0; i < format_ctx->nb_streams; i++) {
        codec_ctx = format_ctx->streams[i]->codec;
        if (codec_ctx && codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream = format_ctx->streams[i];
            break;
        }
    }
    if (!stream) {
        qDebug("ERROR: couldn't find any audio stream in the file");
        goto done;
    }

    codec = avcodec_find_decoder(codec_ctx->codec_id);
    if (!codec) {
        qDebug("ERROR: unknown codec");
        goto done;
    }

    /* request regular signed 16-bit packed format */
    codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        qDebug("ERROR: couldn't open the codec");
        goto done;
    }
    codec_ctx_opened = 1;

    if (codec_ctx->channels <= 0) {
        qDebug("ERROR: no channels found in the audio stream");
        goto done;
    }

    if (codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
        convert_ctx = av_audio_convert_alloc(AV_SAMPLE_FMT_S16, codec_ctx->channels,
                                             codec_ctx->sample_fmt, codec_ctx->channels, NULL, 0);
        if (!convert_ctx) {
            qDebug("ERROR: couldn't create sample format converter");
            goto done;
        }
    }

    *duration = stream->time_base.num * stream->duration / stream->time_base.den;

    av_init_packet(&packet);
    av_init_packet(&packet_temp);

    remaining = max_length * codec_ctx->channels * codec_ctx->sample_rate;
    chromaprint_start(chromaprint_ctx, codec_ctx->sample_rate, codec_ctx->channels);

    while (1) {
        if (av_read_frame(format_ctx, &packet) < 0) {
            break;
        }

        packet_temp.data = packet.data;
        packet_temp.size = packet.size;

        while (packet_temp.size > 0) {
            buffer_size = BUFFER_SIZE;

            consumed = avcodec_decode_audio3(codec_ctx, buffer1, &buffer_size, &packet_temp);

            if (consumed < 0) {
                break;
            }

            packet_temp.data += consumed;
            packet_temp.size -= consumed;

            if (buffer_size <= 0) {
                if (buffer_size < 0) {
                    qDebug("WARNING: size returned from avcodec_decode_audioX is too small\n");
                }
                continue;
            }
            if (buffer_size > BUFFER_SIZE) {
                qDebug("WARNING: size returned from avcodec_decode_audioX is too large\n");
                continue;
            }

            if (convert_ctx) {
                const void *ibuf[6] = { buffer1 };
                void *obuf[6] = { buffer2 };
                int istride[6] = { av_get_bytes_per_sample(codec_ctx->sample_fmt) };
                int ostride[6] = { 2 };
                int len = buffer_size / istride[0];
                if (av_audio_convert(convert_ctx, obuf, ostride, ibuf, istride, len) < 0) {
                    qDebug("WARNING: unable to convert %d samples\n", buffer_size);
                    break;
                }
                buffer = buffer2;
                buffer_size = len * ostride[0];
            }
            else {
                buffer = buffer1;
            }

            length = MIN(remaining, buffer_size / 2);
            if (!chromaprint_feed(chromaprint_ctx, buffer, length)) {
                qDebug("ERROR: fingerprint calculation failed\n");
                goto done;
            }

            if (max_length) {
                remaining -= length;
                if (remaining <= 0) {
                    goto finish;
                }
            }
        }

        if (packet.data) {
            av_free_packet(&packet);
        }
    }

finish:
    if (!chromaprint_finish(chromaprint_ctx)) {
        qDebug("ERROR: fingerprint calculation failed\n");
        goto done;
    }

    ok = 1;

done:
    if (codec_ctx_opened) {
        avcodec_close(codec_ctx);
    }
    if (format_ctx) {
        avformat_close_input(&format_ctx);
    }

    if (convert_ctx) {
        av_audio_convert_free(convert_ctx);
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
                emit receivedGoodAnswer(titleList, releaseList, artistList);
                emit status(tr("Received good answer...presenting matches...."), 5000);
            }
        } else {
            emit status(tr("Fingerprint could not be matched... :("), 5000);
        }
    } else {
        emit status(tr("Network error - QNetworkReply error code %1").arg(reply->error()), 5000);
    }
    delete reply;
}

void Fingerprinter::networkAccessChanged(QNetworkAccessManager::NetworkAccessibility na) {
    emit status(tr("Network accessibility changed! Network access now %1!").arg((na > 0)?"accessible":"unavailable"), 5000);
}
