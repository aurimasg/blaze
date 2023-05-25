
#include <algorithm>
#include "Benchmark.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>


static constexpr int RunCount = 500;


static constexpr int PNGAdlerBase = 65521;


struct RudimentaryPNGWriter final {
    RudimentaryPNGWriter() {
    }

   ~RudimentaryPNGWriter();

    void Save(const uint8 *bytes, const int width, const int height,
        const int stride, const char *path);
private:
    void WriteRaw(const char *data, const int length);
    void WriteUInt8(const uint8 value);
    void WriteUInt16(const uint16 value);
    void WriteUInt32(const uint32 value);
    void WriteRawUpdateCRC(const char *data, const int length);
    void WriteUInt8UpdateCRC(const uint8 value);
    void WriteUInt16UpdateCRC(const uint16 value);
    void WriteUInt32UpdateCRC(const uint32 value);
    void PixelHeader(const int pos, const int bytesPerLine);
    void WriteAdler(unsigned char data);
    void NewChunk(const char *name, const int length);
    void EndChunk();
    void FlushToContainNewData(const int newDataSize);
private:
    uint8 *mData = nullptr;
    int mDataLength = 0;
    int mDataCapacity = 0;
    int mFile = -1;
    uint32 mCRC = 0;
    uint16 mA1 = 1;
    uint16 mA2 = 0;
private:
    DISABLE_COPY_AND_ASSIGN(RudimentaryPNGWriter);
};


static const uint32 PNGCRC32[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
    0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
    0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
    0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
    0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
    0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
    0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
    0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
    0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
    0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
    0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


RudimentaryPNGWriter::~RudimentaryPNGWriter()
{
    ASSERT(mFile == -1);

    free(mData);
}


static constexpr uint32 SwapBytes(const uint32 v) {
    return ((v >> 24) & 0x000000ff) |
           ((v << 8)  & 0x00ff0000) |
           ((v >> 8)  & 0x0000ff00) |
           ((v << 24) & 0xff000000);
}


static uint32 UpdateCRC32(const unsigned char *data, const int length,
    uint32 crc)
{
    for (int i = 0; i < length; i++) {
        crc = PNGCRC32[(crc ^ data[i]) & 255] ^ (crc >> 8);
    }

    return crc;
}


void RudimentaryPNGWriter::WriteRaw(const char *data, const int length)
{
    ASSERT(data != nullptr);

    FlushToContainNewData(length);

    memcpy(mData + mDataLength, data, length);

    mDataLength += length;
}


void RudimentaryPNGWriter::WriteUInt8(const uint8 value)
{
    FlushToContainNewData(1);

    uint8 *d = reinterpret_cast<uint8 *>(mData + mDataLength);

    d[0] = value;

    mDataLength++;
}


void RudimentaryPNGWriter::WriteUInt16(const uint16 value)
{
    FlushToContainNewData(2);

    uint16 *d = reinterpret_cast<uint16 *>(mData + mDataLength);

    d[0] = value;

    mDataLength += 2;
}


void RudimentaryPNGWriter::WriteUInt32(const uint32 value)
{
    FlushToContainNewData(4);

    uint32 *d = reinterpret_cast<uint32 *>(mData + mDataLength);

    d[0] = value;

    mDataLength += 4;
}


void RudimentaryPNGWriter::NewChunk(const char *name, const int length)
{
    mCRC = 0xffffffff;

    WriteUInt32(SwapBytes(uint32(length)));

    mCRC = UpdateCRC32(reinterpret_cast<const unsigned char *>(name), 4, mCRC);

    WriteRaw(name, 4);
}


void RudimentaryPNGWriter::EndChunk()
{
    WriteUInt32(SwapBytes(~mCRC));
}


void RudimentaryPNGWriter::WriteUInt32UpdateCRC(const uint32 val)
{
    mCRC = UpdateCRC32(reinterpret_cast<const unsigned char *>(&val), 4, mCRC);

    WriteUInt32(val);
}


void RudimentaryPNGWriter::WriteUInt16UpdateCRC(const uint16 val)
{
    mCRC = UpdateCRC32(reinterpret_cast<const unsigned char *>(&val), 2, mCRC);

    WriteUInt16(val);
}


void RudimentaryPNGWriter::WriteUInt8UpdateCRC(const uint8 val)
{
    mCRC = UpdateCRC32(reinterpret_cast<const unsigned char *>(&val), 1, mCRC);

    WriteUInt8(val);
}

void RudimentaryPNGWriter::WriteRawUpdateCRC(const char *data,
    const int length)
{
    mCRC = UpdateCRC32(reinterpret_cast<const unsigned char *>(data), length,
        mCRC);

    WriteRaw(data, length);
}


void RudimentaryPNGWriter::WriteAdler(unsigned char data)
{
    WriteRawUpdateCRC(reinterpret_cast<char *>(&data), 1);

    mA1 = uint16((mA1 + data) % PNGAdlerBase);
    mA2 = uint16((mA2 + mA1) % PNGAdlerBase);
}


void RudimentaryPNGWriter::PixelHeader(const int pos, const int bytesPerLine)
{
    if (pos > bytesPerLine) {
        WriteRawUpdateCRC("\0", 1);

        WriteUInt16UpdateCRC(uint16(bytesPerLine));
        WriteUInt16UpdateCRC(uint16(~bytesPerLine));
    } else {
        WriteRawUpdateCRC("\1", 1);

        WriteUInt16UpdateCRC(uint16(pos));
        WriteUInt16UpdateCRC(uint16(~pos));
    }
}


void RudimentaryPNGWriter::Save(const uint8 *bytes, const int width,
    const int height, const int stride, const char *path)
{
    ASSERT(bytes != nullptr);
    ASSERT(width > 0);
    ASSERT(height > 0);
    ASSERT(stride >= width);
    ASSERT(path != nullptr);
    ASSERT(path[0] != 0);

    ASSERT(mFile == -1);

    mFile = open(path, O_WRONLY | O_CREAT);

    WriteRaw("\211PNG\r\n\032\n", 8);

    // IHDR
    NewChunk("IHDR", 13);
    WriteUInt32UpdateCRC(SwapBytes(uint32(width)));
    WriteUInt32UpdateCRC(SwapBytes(uint32(height)));
    WriteUInt8UpdateCRC(8);
    WriteUInt8UpdateCRC(uint8(6));
    WriteUInt8UpdateCRC(0);
    WriteUInt8UpdateCRC(0);
    WriteUInt8UpdateCRC(0);
    EndChunk();

    const int bytesPerLine = 1 + 4 * width;

    if (bytesPerLine >= 65536) {
        // Lines are too long.
        return;
    }

    int rawSize = height * bytesPerLine;

    const int size = 2 + height * (5 + bytesPerLine) + 4;

    NewChunk("IDAT", size);
    WriteRawUpdateCRC("\170\332", 2);

    mCRC = 0;
    mA1 = 1;
    mA2 = 0;

    int index = 0;

    for (int yy = 0; yy < height; yy++) {
        const uint8 *pixel = bytes + (yy * stride);

        for (int xx = 0; xx < width; xx++) {
            if (index == 0) {
                PixelHeader(rawSize, bytesPerLine);

                WriteAdler(0);

                rawSize--;
            }

            WriteAdler(pixel[0]);
            WriteAdler(pixel[1]);
            WriteAdler(pixel[2]);
            WriteAdler(pixel[3]);

            pixel += 4;

            rawSize -= 4;

            index = (index + 1) % width;
        }
    }

    mA1 %= PNGAdlerBase;
    mA2 %= PNGAdlerBase;

    WriteUInt32UpdateCRC(SwapBytes(uint32((mA2 << 16) | mA1)));
    EndChunk();

    NewChunk("IEND", 0);
    EndChunk();

    if (mDataLength > 0) {
        write(mFile, mData, mDataLength);
    }

    close(mFile);

    mFile = -1;
}


void RudimentaryPNGWriter::FlushToContainNewData(const int newDataSize)
{
    const int r = mDataCapacity - mDataLength;

    if (r < newDataSize) {
        if (mDataLength > 0) {
            write(mFile, mData, mDataLength);

            mDataLength = 0;
        }

        if (mDataCapacity < newDataSize) {
            free(mData);

            mDataCapacity = Max(newDataSize, 1024 * 128);

            mData = static_cast<uint8 *>(malloc(mDataCapacity));
        }
    }
}


static void SaveImage(const uint8 *bytes, const int width, const int height,
    const int stride, const char *path)
{
    RudimentaryPNGWriter png;

    png.Save(bytes, width, height, stride, path);
}


static double TimestampInMilliseconds()
{
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return static_cast<double>(milliseconds);
}


double Benchmark::Run(const VectorImage &vg, const double scale, const char *op)
{
    ASSERT(scale > DBL_EPSILON);

    const IntRect bounds = vg.GetBounds();

    const int minx = int(Floor(double(bounds.MinX) * scale));
    const int miny = int(Floor(double(bounds.MinY) * scale));
    const int maxx = int(Ceil(double(bounds.MaxX) * scale));
    const int maxy = int(Ceil(double(bounds.MaxY) * scale));

    const int h = maxx - minx;
    const int v = maxy - miny;

    Matrix matrix = Matrix::CreateScale(scale);
    matrix.PostTranslate(-minx, -miny);

    const int a = h * 4;
    const int bytesPerRow = (a + 127) & ~127;
    const int byteCount = bytesPerRow * v;

    unsigned char *p = static_cast<unsigned char *>(malloc(byteCount));

    const ImageData image(p, h, v, bytesPerRow);

    Prepare(vg.GetGeometries(), vg.GetGeometryCount());

    double times[RunCount];

    for (int i = 0; i < RunCount; i++) {
        memset(p, 0, byteCount);

        const double t0 = TimestampInMilliseconds();

        RenderOnce(matrix, image);

        const double t1 = TimestampInMilliseconds();

        times[i] = t1 - t0;
    }

    SaveImage(p, h, v, bytesPerRow, op);

    free(p);

    std::sort(times, times + RunCount);

    double accumulation = 0;

    for (int i = 5; i < RunCount - 5; i++) {
        accumulation += times[i];
    }

    return accumulation / double(RunCount - 10);
}
