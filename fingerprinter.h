#ifndef FINGERPRINTER_H
#define FINGERPRINTER_H

# include <QString>

# include <chromaprint.h>

class Fingerprinter
{
public:
    Fingerprinter();

    static QString getFingerpint(const QString &filename);

private:
    static int decode_audio_file(ChromaprintContext *chromaprint_ctx, int16_t *buffer1, int16_t *buffer2,
                          const char *file_name, int max_length, int *duration);
};

#endif // FINGERPRINTER_H
