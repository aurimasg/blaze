
#include "BitOps.h"
#include "CompositionOps.h"
#include "IntSize.h"
#include "Linearizer.h"
#include "LineArray.h"
#include "Rasterizer.h"
#include "RasterizerUtils.h"
#include "RowItemList.h"
#include "SIMD.h"


using PixelIndex = uint32;


template <typename T>
struct Rasterizer final {

    static void Rasterize(const Geometry *inputGeometries,
        const int inputGeometryCount, const Matrix &matrix, Threads &threads,
        const ImageData &image);

private:

    static constexpr PixelIndex F24Dot8ToPixelIndex(const F24Dot8 x) {
        return PixelIndex(x >> 8);
    }


    static constexpr F24Dot8 PixelIndexToF24Dot8(const PixelIndex x) {
        return F24Dot8(x) << 8;
    }

private:

    struct RasterizableItem;

    using LineIterationFunction = void (*)(const RasterizableItem *,
        BitVector **, int32 **);

    struct RasterizableGeometry final {
        constexpr RasterizableGeometry(const Geometry *geometry,
            const LineIterationFunction iterationFunction,
            const TileBounds bounds)
        :   Geometry(geometry),
            IterationFunction(iterationFunction),
            Bounds(bounds)
        {
        }

        void *GetLinesForRow(const int rowIndex) const;
        int GetFirstBlockLineCountForRow(const int rowIndex) const;
        const int32 *GetCoversForRow(const int rowIndex) const;
        const int32 *GetActualCoversForRow(const int rowIndex) const;

        const Geometry *Geometry = nullptr;
        const LineIterationFunction IterationFunction = nullptr;
        const TileBounds Bounds;
        void **Lines = nullptr;
        int *FirstBlockLineCounts = nullptr;
        int32 **StartCoverTable = nullptr;
    };


    struct RasterizableItem final {
        RasterizableItem();
        RasterizableItem(const RasterizableGeometry *rasterizable,
            const int localRowIndex);

        int GetFirstBlockLineCount() const;
        const void *GetLineArray() const;
        const int32 *GetActualCovers() const;

        // Do not initialize these since they are allocated in bunches.
        const RasterizableGeometry *Rasterizable;
        int LocalRowIndex;
    };


    static void IterateLinesX32Y16(const RasterizableItem *item,
        BitVector **bitVectorTable, int32 **coverAreaTable);


    static void IterateLinesX16Y16(const RasterizableItem *item,
        BitVector **bitVectorTable, int32 **coverAreaTable);


    static RasterizableGeometry *CreateRasterizable(void *placement,
        const Geometry *geometry, const IntSize imageSize, ThreadMemory &memory);


    template <typename L>
    static RasterizableGeometry *Linearize(void *placement, const Geometry *geometry,
        const TileBounds &bounds, const IntSize imageSize,
        const LineIterationFunction iterationFunction, ThreadMemory &memory);


    static void Vertical_Down(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex columnIndex, const F24Dot8 y0, const F24Dot8 y1, const F24Dot8 x);


    static void Vertical_Up(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex columnIndex, const F24Dot8 y0, const F24Dot8 y1, const F24Dot8 x);


    static void CellVertical(BitVector **bitVectorTable,
        int32 **coverAreaTable, const PixelIndex px, const PixelIndex py,
        const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1);


    static void Cell(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex px, const PixelIndex py, const F24Dot8 x0,
        const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);


    /**
     * ⬊
     *
     * Rasterize line within single pixel row. Line must go from left to
     * right.
     */
    static void RowDownR(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬊
     *
     * Rasterize line within single pixel row. Line must go from left to
     * right or be vertical.
     */
    static void RowDownR_V(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬈
     *
     * Rasterize line within single pixel row. Line must go from left to
     * right.
     */
    static void RowUpR(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬈
     *
     * Rasterize line within single pixel row. Line must go from left to
     * right or be vertical.
     */
    static void RowUpR_V(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬋
     *
     * Rasterize line within single pixel row. Line must go from right to
     * left.
     */
    static void RowDownL(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬋
     *
     * Rasterize line within single pixel row. Line must go from right to
     * left or be vertical.
     */
    static void RowDownL_V(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬉
     *
     * Rasterize line within single pixel row. Line must go from right to
     * left.
     */
    static void RowUpL(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬉
     *
     * Rasterize line within single pixel row. Line must go from right to
     * left or be vertical.
     */
    static void RowUpL_V(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex, const F24Dot8 p0x, const F24Dot8 p0y,
        const F24Dot8 p1x, const F24Dot8 p1y);


    /**
     * ⬊
     */
    static void LineDownR(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex0, const PixelIndex rowIndex1,
        const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1,
        const F24Dot8 y1);


    /**
     * ⬈
     */
    static void LineUpR(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex0, const PixelIndex rowIndex1,
        const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1,
        const F24Dot8 y1);


    /**
     * ⬋
     */
    static void LineDownL(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex0, const PixelIndex rowIndex1,
        const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1,
        const F24Dot8 y1);


    /**
     * ⬉
     */
    static void LineUpL(BitVector **bitVectorTable, int32 **coverAreaTable,
        const PixelIndex rowIndex0, const PixelIndex rowIndex1,
        const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1,
        const F24Dot8 y1);


    static void RasterizeLine(const F24Dot8 X0, const F24Dot8 Y0,
        const F24Dot8 X1, const F24Dot8 Y1, BitVector **bitVectorTable,
        int32 **coverAreaTable);


    template <typename B, FillRuleFn ApplyFillRule>
    static void RenderOneLine(uint8 *image, const BitVector *bitVectorTable,
        const int bitVectorCount, const int32 *coverAreaTable, const int x,
        const int rowLength, const int32 startCover, const uint32 color);


    /**
     * Rasterize one item within a single row.
     */
    static void RasterizeOneItem(const RasterizableItem *item,
        BitVector **bitVectorTable, int32 **coverAreaTable,
        const int columnCount, const ImageData &image);


    /**
     * Rasterize all items in one row.
     */
    static void RasterizeRow(const RowItemList<RasterizableItem> *rowList,
        ThreadMemory &memory, const ImageData &image);

private:
    Rasterizer() = delete;
};


template <typename T>
FORCE_INLINE void Rasterizer<T>::Rasterize(const Geometry *inputGeometries,
    const int inputGeometryCount, const Matrix &matrix, Threads &threads,
    const ImageData &image)
{
    ASSERT(inputGeometries != nullptr);
    ASSERT(inputGeometryCount > 0);
    ASSERT(image.Data != nullptr);
    ASSERT(image.Width > 0);
    ASSERT(image.Height > 0);
    ASSERT(image.BytesPerRow >= (image.Width * 4));

    // TODO
    // Skip transform if matrix is identity.
    Geometry *geometries = static_cast<Geometry *>(
        threads.MallocMain(SIZE_OF(Geometry) * inputGeometryCount));

    for (int i = 0; i < inputGeometryCount; i++) {
        const Geometry *s = inputGeometries + i;

        Matrix tm(s->TM);

        tm.PreMultiply(matrix);

        new (geometries + i) Geometry(
            tm.MapBoundingRect(s->PathBounds),
            s->Tags,
            s->Points,
            tm,
            s->TagCount,
            s->PointCount,
            s->Color,
            s->Rule);
    }

    // Step 1.
    //
    // Create and array of RasterizableGeometry instances. Instances are
    // created and prepared for further processing in parallel.

    // Allocate memory for RasterizableGeometry instance pointers.
    const RasterizableGeometry **rasterizables = static_cast<const RasterizableGeometry **>(
        threads.MallocMain(SIZE_OF(RasterizableGeometry *) * inputGeometryCount));

    // Allocate memory for RasterizableGeometry instances.
    RasterizableGeometry *rasterizableGeometryMemory = static_cast<RasterizableGeometry *>(
        threads.MallocMain(SIZE_OF(RasterizableGeometry) * inputGeometryCount));

    const IntSize imageSize = {
        image.Width,
        image.Height
    };

    threads.ParallelFor(inputGeometryCount, [=](const int index, ThreadMemory &memory) {
        rasterizables[index] = CreateRasterizable(
            rasterizableGeometryMemory + index, geometries + index, imageSize,
            memory);
    });

    // Linearizer may decide that some paths do not contribute to the final
    // image. In these situations CreateRasterizable will return nullptr. In
    // the following step, a new array is created and only non-nullptr items
    // are copied to it.

    const RasterizableGeometry **visibleRasterizables = static_cast<const RasterizableGeometry **>(
        threads.MallocMain(SIZE_OF(RasterizableGeometry *) * inputGeometryCount));

    int visibleRasterizableCount = 0;

    for (int i = 0; i < inputGeometryCount; i++) {
        const RasterizableGeometry *rasterizable = rasterizables[i];

        if (rasterizable != nullptr) {
            visibleRasterizables[visibleRasterizableCount++] = rasterizable;
        }
    }


    // Step 2.
    //
    // Create lists of rasterizable items for each interval.

    const TileIndex rowCount = CalculateRowCount<T>(imageSize.Height);

    RowItemList<RasterizableItem> *rowLists =
        static_cast<RowItemList<RasterizableItem> *>(threads.MallocMain(SIZE_OF(RowItemList<T>) * rowCount));

    const int threadCount = Threads::GetHardwareThreadCount();

    ASSERT(threadCount > 0);

    const int iterationHeight = Max<uint32>(rowCount / threadCount, 1);
    const int iterationCount = (rowCount / iterationHeight) +
        Min<uint32>(rowCount % iterationHeight, 1);

    threads.ParallelFor(iterationCount, [&](const int index, ThreadMemory &memory) {
        const TileIndex threadY = index * iterationHeight;
        const TileIndex threadHeight = Min<TileIndex>(iterationHeight, rowCount - threadY);
        const TileIndex threadMaxY = threadY + threadHeight;

        for (TileIndex i = threadY; i < threadMaxY; i++) {
            new (rowLists + i) RowItemList<RasterizableItem>();
        }

        for (int i = 0; i < visibleRasterizableCount; i++) {
            const RasterizableGeometry *rasterizable = visibleRasterizables[i];
            const TileBounds b = rasterizable->Bounds;

            const TileIndex min = Clamp(b.Y, threadY, threadMaxY);
            const TileIndex max = Clamp(b.Y + b.RowCount, threadY,
                threadMaxY);

            if (min == max) {
                continue;
            }

            // Populate all lists which intersect with this geometry.
            for (TileIndex y = min; y < max; y++) {
                // Local row index within row array of geometry.
                const TileIndex localIndex = y - b.Y;

                // There are two situations when this row needs to be
                // inserted. Either it has segments or it has non-zero cover
                // array.
                const bool emptyRow =
                    rasterizable->GetLinesForRow(localIndex) == nullptr and
                    rasterizable->GetCoversForRow(localIndex) == nullptr;

                if (emptyRow) {
                    // Both conditions failed, this geometry for this row will
                    // not produce any visible pixels.
                    continue;
                }

                RowItemList<RasterizableItem> *list = rowLists + y;

                list->Append(memory, rasterizable, localIndex);
            }
        }
    });

    // Step 3.
    //
    // Rasterize all intervals.

    threads.ParallelFor(rowCount, [&](const int rowIndex, ThreadMemory &memory) {
        const RowItemList<RasterizableItem> *item = rowLists + rowIndex;

        RasterizeRow(item, memory, image);
    });
}


template <typename T>
FORCE_INLINE void *Rasterizer<T>::RasterizableGeometry::GetLinesForRow(const int rowIndex) const {
    ASSERT(rowIndex >= 0);
    ASSERT(rowIndex < Bounds.RowCount);

    return Lines[rowIndex];
}


template <typename T>
FORCE_INLINE int Rasterizer<T>::RasterizableGeometry::GetFirstBlockLineCountForRow(const int rowIndex) const {
    ASSERT(rowIndex >= 0);
    ASSERT(rowIndex < Bounds.RowCount);

    return FirstBlockLineCounts[rowIndex];
}


template <typename T>
FORCE_INLINE const int32 *Rasterizer<T>::RasterizableGeometry::GetCoversForRow(const int rowIndex) const {
    ASSERT(rowIndex >= 0);
    ASSERT(rowIndex < Bounds.RowCount);

    if (StartCoverTable == nullptr) {
        // No table at all.
        return nullptr;
    }

    return StartCoverTable[rowIndex];
}


template <typename T>
FORCE_INLINE const int32 *Rasterizer<T>::RasterizableGeometry::GetActualCoversForRow(const int rowIndex) const {
    ASSERT(rowIndex >= 0);
    ASSERT(rowIndex < Bounds.RowCount);

    if (StartCoverTable == nullptr) {
        // No table at all.
        return T::ZeroCovers;
    }

    const int32 *covers = StartCoverTable[rowIndex];

    if (covers == nullptr) {
        return T::ZeroCovers;
    }

    return covers;
}


template <typename T>
FORCE_INLINE Rasterizer<T>::RasterizableItem::RasterizableItem() {
}


template <typename T>
FORCE_INLINE Rasterizer<T>::RasterizableItem::RasterizableItem(const RasterizableGeometry *rasterizable,
    const int localRowIndex)
:   Rasterizable(rasterizable),
    LocalRowIndex(localRowIndex)
{
}


template <typename T>
FORCE_INLINE int Rasterizer<T>::RasterizableItem::GetFirstBlockLineCount() const  {
    return Rasterizable->GetFirstBlockLineCountForRow(LocalRowIndex);
}


template <typename T>
FORCE_INLINE const void *Rasterizer<T>::RasterizableItem::GetLineArray() const {
    return Rasterizable->GetLinesForRow(LocalRowIndex);
}


template <typename T>
FORCE_INLINE const int32 *Rasterizer<T>::RasterizableItem::GetActualCovers() const {
    return Rasterizable->GetActualCoversForRow(LocalRowIndex);
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::IterateLinesX32Y16(const RasterizableItem *item, BitVector **bitVectorTable, int32 **coverAreaTable) {
    int count = item->GetFirstBlockLineCount();

    const LineArrayX32Y16::Block *v =
        static_cast<const LineArrayX32Y16::Block *>(item->GetLineArray());

    while (v != nullptr) {
        const F8Dot8x2 *yy = v->Y0Y1;
        const F24Dot8 *xx0 = v->X0;
        const F24Dot8 *xx1 = v->X1;

        for (int i = 0; i < count; i++) {
            const F8Dot8x2 y0y1 = yy[i];
            const F24Dot8 x0 = xx0[i];
            const F24Dot8 x1 = xx1[i];

            RasterizeLine(x0, UnpackLoFromF8Dot8x2(y0y1), x1,
                UnpackHiFromF8Dot8x2(y0y1), bitVectorTable,
                coverAreaTable);
        }

        v = v->Next;
        count = LineArrayX32Y16::Block::LinesPerBlock;
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::IterateLinesX16Y16(const RasterizableItem *item, BitVector **bitVectorTable, int32 **coverAreaTable) {
    int count = item->GetFirstBlockLineCount();

    const LineArrayX16Y16::Block *v =
        static_cast<const LineArrayX16Y16::Block *>(item->GetLineArray());

    while (v != nullptr) {
        const F8Dot8x2 *yy = v->Y0Y1;
        const F8Dot8x2 *xx = v->X0X1;

        for (int i = 0; i < count; i++) {
            const F8Dot8x2 y0y1 = yy[i];
            const F8Dot8x2 x0x1 = xx[i];

            RasterizeLine(
                UnpackLoFromF8Dot8x2(x0x1),
                UnpackLoFromF8Dot8x2(y0y1),
                UnpackHiFromF8Dot8x2(x0x1),
                UnpackHiFromF8Dot8x2(y0y1), bitVectorTable,
                coverAreaTable);
        }

        v = v->Next;
        count = LineArrayX32Y16::Block::LinesPerBlock;
    }
}


template <typename T>
FORCE_INLINE typename Rasterizer<T>::RasterizableGeometry *
Rasterizer<T>::CreateRasterizable(void *placement, const Geometry *geometry, const IntSize imageSize, ThreadMemory &memory) {
    ASSERT(placement != nullptr);
    ASSERT(geometry != nullptr);
    ASSERT(imageSize.Width > 0);
    ASSERT(imageSize.Height > 0);

    if (geometry->TagCount < 1) {
        return nullptr;
    }

    // Path bounds in geometry are transformed by transformation matrix, but
    // not intersected with destination image bounds (path bounds can be
    // bigger than destination image bounds).
    //
    // Next step is to intersect transformed path bounds with destination
    // image bounds and see if there is something left.
    //
    // Note that there is a special consideration regarding maximum X path
    // bounding box edge. Consider path representing a rectangle. Vertical
    // line going from top to bottom at the right edge of path bounding box.
    // This line should close rectangle. But line clipper simply ignores it
    // because it ignores all lines that have X coordinates equal to or to the
    // right of path bounding box. As a result, this path is then drawn to the
    // edge of destination image instead of terminating at the right rectangle
    // edge.
    //
    // To solve this problem 1 is added to the maximum X coordinate of path
    // bounding box to allow inserting vertical lines at the right edge of
    // path bounding box so shapes get a chance to terminate fill. Perhaps
    // there are better ways to solve this (maybe clipper should not ignore
    // lines at the maximum X edge of path bounds?), but for now I'm keeping
    // this fix.

    const IntRect geometryBounds = geometry->PathBounds;

    if (geometryBounds.MinX == geometryBounds.MaxX) {
        return nullptr;
    }

    const int minx = Max(0, geometryBounds.MinX);
    const int miny = Max(0, geometryBounds.MinY);
    const int maxx = Min(imageSize.Width, geometryBounds.MaxX + 1);
    const int maxy = Min(imageSize.Height, geometryBounds.MaxY);

    if (minx >= maxx or miny >= maxy) {
        // Geometry bounds do not intersect with destination image.
        return nullptr;
    }

    const TileBounds bounds = CalculateTileBounds<T>(minx, miny, maxx, maxy);

    const bool narrow =
        128 > (bounds.ColumnCount * T::TileW);

    if (narrow) {
        return Linearize<LineArrayX16Y16>(placement, geometry, bounds,
            imageSize, IterateLinesX16Y16, memory);
    } else {
        return Linearize<LineArrayX32Y16>(placement, geometry, bounds,
            imageSize, IterateLinesX32Y16, memory);
    }
}


template <typename T>
template <typename L>
FORCE_INLINE typename Rasterizer<T>::RasterizableGeometry *
Rasterizer<T>::Linearize(void *placement, const Geometry *geometry, const TileBounds &bounds, const IntSize imageSize, const LineIterationFunction iterationFunction, ThreadMemory &memory) {
    RasterizableGeometry *linearized = new (placement) RasterizableGeometry(
        geometry, iterationFunction, bounds);

    // Determine if path is completely within destination image bounds. If
    // geometry bounds fit within destination image, a shortcut can be made
    // when generating lines.
    const bool contains =
        geometry->PathBounds.MinX >= 0 and
        geometry->PathBounds.MinY >= 0 and
        geometry->PathBounds.MaxX <= imageSize.Width and
        geometry->PathBounds.MaxY <= imageSize.Height;

    Linearizer<T, L> *linearizer =
        Linearizer<T, L>::Create(memory, bounds, contains, geometry);

    ASSERT(linearizer != nullptr);

    // Finalize.
    void **lineBlocks = memory.FrameMallocArray<void *>(bounds.RowCount);

    int32 *firstLineBlockCounts = memory.FrameMallocArray<int32>(
        bounds.RowCount);

    for (int i = 0; i < bounds.RowCount; i++) {
        const L *la = linearizer->GetLineArrayAtIndex(i);

        ASSERT(la != nullptr);

        if (la->GetFrontBlock() == nullptr) {
            lineBlocks[i] = nullptr;
            firstLineBlockCounts[i] = 0;
            continue;
        }

        lineBlocks[i] = la->GetFrontBlock();
        firstLineBlockCounts[i] = la->GetFrontBlockLineCount();
    }

    linearized->Lines = lineBlocks;
    linearized->FirstBlockLineCounts = firstLineBlockCounts;

    int32 **startCoverTable = linearizer->GetStartCoverTable();

    if (startCoverTable != nullptr) {
        for (int i = 0; i < bounds.RowCount; i++) {
            const int32 *t = startCoverTable[i];

            if (t != nullptr and T::CoverArrayContainsOnlyZeroes(t)) {
                // Don't need cover array after all, all segments cancelled
                // each other.
                startCoverTable[i] = nullptr;
            }
        }

        linearized->StartCoverTable = startCoverTable;
    }

    return linearized;
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::Vertical_Down(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex columnIndex, const F24Dot8 y0,
    const F24Dot8 y1, const F24Dot8 x)
{
    ASSERT(y0 < y1);

    const PixelIndex rowIndex0 = F24Dot8ToPixelIndex(y0);
    const PixelIndex rowIndex1 = F24Dot8ToPixelIndex(y1 - 1);
    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);
    const F24Dot8 fx = x - PixelIndexToF24Dot8(columnIndex);

    if (rowIndex0 == rowIndex1) {
        return CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex0, fx, fy0, fy1);
    } else {
        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex0, fx, fy0, F24Dot8_1);

        for (PixelIndex i = rowIndex0 + 1; i < rowIndex1; i++) {
            CellVertical(bitVectorTable, coverAreaTable, columnIndex, i, fx, 0, F24Dot8_1);
        }

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex1, fx, 0, fy1);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::Vertical_Up(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex columnIndex, const F24Dot8 y0,
    const F24Dot8 y1, const F24Dot8 x)
{
    ASSERT(y0 > y1);

    const PixelIndex rowIndex0 = F24Dot8ToPixelIndex(y0 - 1);
    const PixelIndex rowIndex1 = F24Dot8ToPixelIndex(y1);
    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);
    const F24Dot8 fx = x - PixelIndexToF24Dot8(columnIndex);

    if (rowIndex0 == rowIndex1) {
        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex0, fx, fy0, fy1);
    } else {
        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex0, fx, fy0, 0);

        for (PixelIndex i = rowIndex0 - 1; i > rowIndex1; i--) {
            CellVertical(bitVectorTable, coverAreaTable, columnIndex, i, fx, F24Dot8_1, 0);
        }

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex1, fx, F24Dot8_1, fy1);
    }
}


template <typename T>
void Rasterizer<T>::CellVertical(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex px, const PixelIndex py,
    const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1)
{
    ASSERT(px >= 0);
    ASSERT(py >= 0);
    ASSERT(py < T::TileH);

    const F24Dot8 delta = y0 - y1;
    const F24Dot8 a = delta * (F24Dot8_2 - x - x);
    const int index = px << 1;
    int32 *ca = coverAreaTable[py];

    if (ConditionalSetBit(bitVectorTable[py], px)) {
        // New.
        ca[index] = delta;
        ca[index + 1] = a;
    } else {
        // Update old.
        const int32 cover = ca[index];
        const int32 area = ca[index + 1];

        ca[index] = cover + delta;
        ca[index + 1] = area + a;
    }
}


template <typename T>
void Rasterizer<T>::Cell(BitVector **bitVectorTable, int32 **coverAreaTable,
    const PixelIndex px, const PixelIndex py, const F24Dot8 x0,
    const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1)
{
    ASSERT(px >= 0);
    ASSERT(py >= 0);
    ASSERT(py < T::TileH);

    const F24Dot8 delta = y0 - y1;
    const F24Dot8 a = delta * (F24Dot8_2 - x0 - x1);
    const int index = px << 1;
    int32 *ca = coverAreaTable[py];

    if (ConditionalSetBit(bitVectorTable[py], px)) {
        // New.
        ca[index] = delta;
        ca[index + 1] = a;
    } else {
        // Update old.
        const int32 cover = ca[index];
        const int32 area = ca[index + 1];

        ca[index] = cover + delta;
        ca[index + 1] = area + a;
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowDownR(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x < p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    const PixelIndex columnIndex0 = F24Dot8ToPixelIndex(p0x);
    const PixelIndex columnIndex1 = F24Dot8ToPixelIndex(p1x - 1);

    ASSERT(columnIndex0 <= columnIndex1);

    // Extract remainders.
    const F24Dot8 fx0 = p0x - PixelIndexToF24Dot8(columnIndex0);
    const F24Dot8 fx1 = p1x - PixelIndexToF24Dot8(columnIndex1);

    ASSERT(fx0 >= 0);
    ASSERT(fx0 <= F24Dot8_1);
    ASSERT(fx1 >= 0);
    ASSERT(fx1 <= F24Dot8_1);

    if (columnIndex0 == columnIndex1) {
        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, fx1, p1y);
    } else {
        // Horizontal and vertical deltas.
        const F24Dot8 dx = p1x - p0x;
        const F24Dot8 dy = p1y - p0y;

        const F24Dot8 pp = (F24Dot8_1 - fx0) * dy;

        F24Dot8 cy = p0y + (pp / dx);

        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y,
            F24Dot8_1, cy);

        PixelIndex idx = columnIndex0 + 1;

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = F24Dot8_1 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx++) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy + delta;

                Cell(bitVectorTable, coverAreaTable, idx, rowIndex, 0, cy,
                    F24Dot8_1, ny);

                cy = ny;
            }
        }

        Cell(bitVectorTable, coverAreaTable, columnIndex1, rowIndex, 0, cy, fx1, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowDownR_V(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    if (LIKELY(p0x < p1x)) {
        RowDownR(bitVectorTable, coverAreaTable, rowIndex, p0x, p0y, p1x, p1y);
    } else {
        const PixelIndex columnIndex = F24Dot8ToPixelIndex(p0x - FindAdjustment(p0x));
        const F24Dot8 x = p0x - PixelIndexToF24Dot8(columnIndex);

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex, x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowUpR(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x < p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    const PixelIndex columnIndex0 = F24Dot8ToPixelIndex(p0x);
    const PixelIndex columnIndex1 = F24Dot8ToPixelIndex(p1x - 1);

    ASSERT(columnIndex0 <= columnIndex1);

    // Extract remainders.
    const F24Dot8 fx0 = p0x - PixelIndexToF24Dot8(columnIndex0);
    const F24Dot8 fx1 = p1x - PixelIndexToF24Dot8(columnIndex1);

    ASSERT(fx0 >= 0);
    ASSERT(fx0 <= F24Dot8_1);
    ASSERT(fx1 >= 0);
    ASSERT(fx1 <= F24Dot8_1);

    if (columnIndex0 == columnIndex1) {
        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, fx1, p1y);
    } else {
        // Horizontal and vertical deltas.
        const F24Dot8 dx = p1x - p0x;
        const F24Dot8 dy = p0y - p1y;

        const F24Dot8 pp = (F24Dot8_1 - fx0) * dy;

        F24Dot8 cy = p0y - (pp / dx);

        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y,
            F24Dot8_1, cy);

        PixelIndex idx = columnIndex0 + 1;

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = F24Dot8_1 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx++) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy - delta;

                Cell(bitVectorTable, coverAreaTable, idx, rowIndex, 0, cy,
                    F24Dot8_1, ny);

                cy = ny;
            }
        }

        Cell(bitVectorTable, coverAreaTable, columnIndex1, rowIndex, 0, cy, fx1, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowUpR_V(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    if (LIKELY(p0x < p1x)) {
        RowUpR(bitVectorTable, coverAreaTable, rowIndex, p0x, p0y, p1x, p1y);
    } else {
        const PixelIndex columnIndex = F24Dot8ToPixelIndex(p0x - FindAdjustment(p0x));
        const F24Dot8 x = p0x - PixelIndexToF24Dot8(columnIndex);

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex, x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowDownL(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x > p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    const PixelIndex columnIndex0 = F24Dot8ToPixelIndex(p0x - 1);
    const PixelIndex columnIndex1 = F24Dot8ToPixelIndex(p1x);

    ASSERT(columnIndex1 <= columnIndex0);

    // Extract remainders.
    const F24Dot8 fx0 = p0x - PixelIndexToF24Dot8(columnIndex0);
    const F24Dot8 fx1 = p1x - PixelIndexToF24Dot8(columnIndex1);

    ASSERT(fx0 >= 0);
    ASSERT(fx0 <= F24Dot8_1);
    ASSERT(fx1 >= 0);
    ASSERT(fx1 <= F24Dot8_1);

    if (columnIndex0 == columnIndex1) {
        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, fx1, p1y);
    } else {
        // Horizontal and vertical deltas.
        const F24Dot8 dx = p0x - p1x;
        const F24Dot8 dy = p1y - p0y;

        const F24Dot8 pp = fx0 * dy;

        F24Dot8 cy = p0y + (pp / dx);

        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, 0, cy);

        PixelIndex idx = columnIndex0 - 1;

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = F24Dot8_1 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx--) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy + delta;

                Cell(bitVectorTable, coverAreaTable, idx, rowIndex, F24Dot8_1,
                    cy, 0, ny);

                cy = ny;
            }
        }

        Cell(bitVectorTable, coverAreaTable, columnIndex1, rowIndex, F24Dot8_1, cy, fx1, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowDownL_V(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    if (LIKELY(p0x > p1x)) {
        RowDownL(bitVectorTable, coverAreaTable, rowIndex, p0x, p0y, p1x, p1y);
    } else {
        const PixelIndex columnIndex = F24Dot8ToPixelIndex(p0x - FindAdjustment(p0x));
        const F24Dot8 x = p0x - PixelIndexToF24Dot8(columnIndex);

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex, x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowUpL(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x > p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    const PixelIndex columnIndex0 = F24Dot8ToPixelIndex(p0x - 1);
    const PixelIndex columnIndex1 = F24Dot8ToPixelIndex(p1x);

    ASSERT(columnIndex1 <= columnIndex0);

    // Extract remainders.
    const F24Dot8 fx0 = p0x - PixelIndexToF24Dot8(columnIndex0);
    const F24Dot8 fx1 = p1x - PixelIndexToF24Dot8(columnIndex1);

    ASSERT(fx0 >= 0);
    ASSERT(fx0 <= F24Dot8_1);
    ASSERT(fx1 >= 0);
    ASSERT(fx1 <= F24Dot8_1);

    if (columnIndex0 == columnIndex1) {
        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, fx1, p1y);
    } else {
        // Horizontal and vertical deltas.
        const F24Dot8 dx = p0x - p1x;
        const F24Dot8 dy = p0y - p1y;

        const F24Dot8 pp = fx0 * dy;

        F24Dot8 cy = p0y - (pp / dx);

        Cell(bitVectorTable, coverAreaTable, columnIndex0, rowIndex, fx0, p0y, 0, cy);

        PixelIndex idx = columnIndex0 - 1;

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = F24Dot8_1 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx--) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy - delta;

                Cell(bitVectorTable, coverAreaTable, idx, rowIndex, F24Dot8_1,
                    cy, 0, ny);

                cy = ny;
            }
        }

        Cell(bitVectorTable, coverAreaTable, columnIndex1, rowIndex, F24Dot8_1, cy, fx1, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RowUpL_V(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex, const F24Dot8 p0x,
    const F24Dot8 p0y, const F24Dot8 p1x, const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    if (LIKELY(p0x > p1x)) {
        RowUpL(bitVectorTable, coverAreaTable, rowIndex, p0x, p0y, p1x, p1y);
    } else {
        const PixelIndex columnIndex = F24Dot8ToPixelIndex(p0x - FindAdjustment(p0x));
        const F24Dot8 x = p0x - PixelIndexToF24Dot8(columnIndex);

        CellVertical(bitVectorTable, coverAreaTable, columnIndex, rowIndex, x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::LineDownR(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex0,
    const PixelIndex rowIndex1, const F24Dot8 x0, const F24Dot8 y0,
    const F24Dot8 x1, const F24Dot8 y1)
{
    ASSERT(y0 < y1);
    ASSERT(x0 < x1);
    ASSERT(rowIndex0 < rowIndex1);

    const F24Dot8 dx = x1 - x0;
    const F24Dot8 dy = y1 - y0;

    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);

    F24Dot8 p = (F24Dot8_1 - fy0) * dx;
    F24Dot8 delta = p / dy;

    F24Dot8 cx = x0 + delta;

    RowDownR_V(bitVectorTable, coverAreaTable, rowIndex0, x0, fy0, cx,
        F24Dot8_1);

    PixelIndex idy = rowIndex0 + 1;

    if (idy != rowIndex1) {
        F24Dot8 mod = (p % dy) - dy;

        p = F24Dot8_1 * dx;

        F24Dot8 lift = p / dy;
        F24Dot8 rem = p % dy;

        for ( ; idy != rowIndex1; idy++) {
            delta = lift;
            mod += rem;

            if (mod >= 0) {
                mod -= dy;
                delta++;
            }

            const F24Dot8 nx = cx + delta;

            RowDownR_V(bitVectorTable, coverAreaTable, idy, cx, 0, nx,
                F24Dot8_1);

            cx = nx;
        }
    }

    RowDownR_V(bitVectorTable, coverAreaTable, rowIndex1, cx, 0, x1, fy1);
}


/**
 * ⬈
 */
template <typename T>
FORCE_INLINE void Rasterizer<T>::LineUpR(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex0,
    const PixelIndex rowIndex1, const F24Dot8 x0, const F24Dot8 y0,
    const F24Dot8 x1, const F24Dot8 y1)
{
    ASSERT(y0 > y1);
    ASSERT(x0 < x1);
    ASSERT(rowIndex0 > rowIndex1);

    const F24Dot8 dx = x1 - x0;
    const F24Dot8 dy = y0 - y1;

    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);

    F24Dot8 p = fy0 * dx;
    F24Dot8 delta = p / dy;

    F24Dot8 cx = x0 + delta;

    RowUpR_V(bitVectorTable, coverAreaTable, rowIndex0, x0, fy0, cx, 0);

    PixelIndex idy = rowIndex0 - 1;

    if (idy != rowIndex1) {
        F24Dot8 mod = (p % dy) - dy;

        p = F24Dot8_1 * dx;

        F24Dot8 lift = p / dy;
        F24Dot8 rem = p % dy;

        for ( ; idy != rowIndex1; idy--) {
            delta = lift;
            mod += rem;

            if (mod >= 0) {
                mod -= dy;
                delta++;
            }

            const F24Dot8 nx = cx + delta;

            RowUpR_V(bitVectorTable, coverAreaTable, idy, cx, F24Dot8_1, nx, 0);

            cx = nx;
        }
    }

    RowUpR_V(bitVectorTable, coverAreaTable, rowIndex1, cx, F24Dot8_1, x1, fy1);
}


/**
 * ⬋
 */
template <typename T>
FORCE_INLINE void Rasterizer<T>::LineDownL(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex0,
    const PixelIndex rowIndex1, const F24Dot8 x0, const F24Dot8 y0,
    const F24Dot8 x1, const F24Dot8 y1)
{
    ASSERT(y0 < y1);
    ASSERT(x0 > x1);
    ASSERT(rowIndex0 < rowIndex1);

    const F24Dot8 dx = x0 - x1;
    const F24Dot8 dy = y1 - y0;

    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);

    F24Dot8 p = (F24Dot8_1 - fy0) * dx;
    F24Dot8 delta = p / dy;

    F24Dot8 cx = x0 - delta;

    RowDownL_V(bitVectorTable, coverAreaTable, rowIndex0, x0, fy0, cx,
        F24Dot8_1);

    PixelIndex idy = rowIndex0 + 1;

    if (idy != rowIndex1) {
        F24Dot8 mod = (p % dy) - dy;

        p = F24Dot8_1 * dx;

        F24Dot8 lift = p / dy;
        F24Dot8 rem = p % dy;

        for ( ; idy != rowIndex1; idy++) {
            delta = lift;
            mod += rem;

            if (mod >= 0) {
                mod -= dy;
                delta++;
            }

            const F24Dot8 nx = cx - delta;

            RowDownL_V(bitVectorTable, coverAreaTable, idy, cx, 0, nx,
                F24Dot8_1);

            cx = nx;
        }
    }

    RowDownL_V(bitVectorTable, coverAreaTable, rowIndex1, cx, 0, x1, fy1);
}


/**
 * ⬉
 */
template <typename T>
FORCE_INLINE void Rasterizer<T>::LineUpL(BitVector **bitVectorTable,
    int32 **coverAreaTable, const PixelIndex rowIndex0,
    const PixelIndex rowIndex1, const F24Dot8 x0, const F24Dot8 y0,
    const F24Dot8 x1, const F24Dot8 y1)
{
    ASSERT(y0 > y1);
    ASSERT(x0 > x1);
    ASSERT(rowIndex0 > rowIndex1);

    const F24Dot8 dx = x0 - x1;
    const F24Dot8 dy = y0 - y1;

    const F24Dot8 fy0 = y0 - PixelIndexToF24Dot8(rowIndex0);
    const F24Dot8 fy1 = y1 - PixelIndexToF24Dot8(rowIndex1);

    F24Dot8 p = fy0 * dx;
    F24Dot8 delta = p / dy;

    F24Dot8 cx = x0 - delta;

    RowUpL_V(bitVectorTable, coverAreaTable, rowIndex0, x0, fy0, cx, 0);

    PixelIndex idy = rowIndex0 - 1;

    if (idy != rowIndex1) {
        F24Dot8 mod = (p % dy) - dy;

        p = F24Dot8_1 * dx;

        F24Dot8 lift = p / dy;
        F24Dot8 rem = p % dy;

        for ( ; idy != rowIndex1; idy--) {
            delta = lift;
            mod += rem;

            if (mod >= 0) {
                mod -= dy;
                delta++;
            }

            const F24Dot8 nx = cx - delta;

            RowUpL_V(bitVectorTable, coverAreaTable, idy, cx, F24Dot8_1, nx, 0);

            cx = nx;
        }
    }

    RowUpL_V(bitVectorTable, coverAreaTable, rowIndex1, cx, F24Dot8_1, x1, fy1);
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RasterizeLine(const F24Dot8 X0,
    const F24Dot8 Y0, const F24Dot8 X1, const F24Dot8 Y1,
    BitVector **bitVectorTable, int32 **coverAreaTable)
{
    ASSERT(Y0 != Y1);
    ASSERT(bitVectorTable != nullptr);
    ASSERT(coverAreaTable != nullptr);

    if (X0 == X1) {
        const PixelIndex columnIndex = F24Dot8ToPixelIndex(X0 - FindAdjustment(X0));

        // Special case, vertical line, simplifies this thing a lot.
        if (Y0 < Y1) {
            // Line is going down ↓
            return Vertical_Down(bitVectorTable, coverAreaTable, columnIndex, Y0, Y1, X0);
        } else {
            // Line is going up ↑
            return Vertical_Up(bitVectorTable, coverAreaTable, columnIndex, Y0, Y1, X0);
        }
    }

    if (Y0 < Y1) {
        // Line is going down ↓
        const PixelIndex rowIndex0 = F24Dot8ToPixelIndex(Y0);
        const PixelIndex rowIndex1 = F24Dot8ToPixelIndex(Y1 - 1);

        ASSERT(rowIndex0 <= rowIndex1);

        if (rowIndex0 == rowIndex1) {
            // Entire line is completely within horizontal band. For curves
            // this is common case.
            const F24Dot8 ty = PixelIndexToF24Dot8(rowIndex0);
            const F24Dot8 y0 = Y0 - ty;
            const F24Dot8 y1 = Y1 - ty;

            if (X0 < X1) {
                return RowDownR(bitVectorTable, coverAreaTable, rowIndex0,
                    X0, y0, X1, y1);
            } else {
                return RowDownL(bitVectorTable, coverAreaTable, rowIndex0,
                    X0, y0, X1, y1);
            }
        } else if (X0 < X1) {
            // Line is going from left to right →
            return LineDownR(bitVectorTable, coverAreaTable, rowIndex0,
                rowIndex1, X0, Y0, X1, Y1);
        } else {
            // Line is going right to left ←
            return LineDownL(bitVectorTable, coverAreaTable, rowIndex0,
                rowIndex1, X0, Y0, X1, Y1);
        }
    } else {
        // Line is going up ↑
        const PixelIndex rowIndex0 = F24Dot8ToPixelIndex(Y0 - 1);
        const PixelIndex rowIndex1 = F24Dot8ToPixelIndex(Y1);

        ASSERT(rowIndex1 <= rowIndex0);

        if (rowIndex0 == rowIndex1) {
            // Entire line is completely within horizontal band. For curves
            // this is common case.
            const F24Dot8 ty = PixelIndexToF24Dot8(rowIndex0);
            const F24Dot8 y0 = Y0 - ty;
            const F24Dot8 y1 = Y1 - ty;

            if (X0 < X1) {
                return RowUpR(bitVectorTable, coverAreaTable, rowIndex0,
                    X0, y0, X1, y1);
            } else {
                return RowUpL(bitVectorTable, coverAreaTable, rowIndex0,
                    X0, y0, X1, y1);
            }
        } else if (X0 < X1) {
            // Line is going from left to right →
            return LineUpR(bitVectorTable, coverAreaTable, rowIndex0,
                rowIndex1, X0, Y0, X1, Y1);
        } else {
            // Line is going right to left ←
            return LineUpL(bitVectorTable, coverAreaTable, rowIndex0,
                rowIndex1, X0, Y0, X1, Y1);
        }
    }
}


template <typename T>
template <typename B, FillRuleFn ApplyFillRule>
FORCE_INLINE void Rasterizer<T>::RenderOneLine(uint8 *image,
    const BitVector *bitVectorTable, const int bitVectorCount,
    const int32 *coverAreaTable, const int x, const int rowLength,
    const int32 startCover, const uint32 color)
{
    ASSERT(image != nullptr);
    ASSERT(bitVectorTable != nullptr);
    ASSERT(bitVectorCount > 0);
    ASSERT(coverAreaTable != nullptr);
    ASSERT(rowLength > 0);

    // X must be aligned on tile boundary.
    ASSERT((x & (T::TileW - 1)) == 0);

    const B blender(color);

    uint32 *d = reinterpret_cast<uint32 *>(image);

    // Cover accumulation.
    int32 cover = startCover;

    // Span state.
    uint32 spanX = x;
    uint32 spanEnd = x;
    uint32 spanAlpha = 0;

    for (uint32 i = 0; i < bitVectorCount; i++) {
        BitVector bitset = bitVectorTable[i];

        while (bitset != 0) {
            const BitVector t = bitset & -bitset;
            const uint32 r = CountTrailingZeroes(bitset);
            const uint32 index = (i * BIT_SIZE_OF(BitVector)) + r;

            bitset ^= t;

            // Note that index is in local geometry coordinates.
            const uint32 tableIndex = index << 1;
            const uint32 edgeX = index + x;
            const uint32 nextEdgeX = edgeX + 1;

            // Signed area for pixel at bit index.
            const int32 area = coverAreaTable[tableIndex + 1] + (cover << 9);

            // Area converted to alpha according to fill rule.
            const uint32 alpha = ApplyFillRule(area);

            if (spanEnd == edgeX) {
                // No gap between previous span and current pixel.
                if (alpha == 0) {
                    if (spanAlpha != 0) {
                        blender.CompositeSpan(spanX, spanEnd, d, spanAlpha);
                    }

                    spanX = nextEdgeX;
                    spanEnd = spanX;
                    spanAlpha = 0;
                } else if (spanAlpha == alpha) {
                    spanEnd = nextEdgeX;
                } else {
                    // Alpha is not zero, but not equal to previous span
                    // alpha.
                    if (spanAlpha != 0) {
                        blender.CompositeSpan(spanX, spanEnd, d, spanAlpha);
                    }

                    spanX = edgeX;
                    spanEnd = nextEdgeX;
                    spanAlpha = alpha;
                }
            } else {
                ASSERT(spanEnd < edgeX);

                // There is a gap between last filled pixel and the new one.
                if (cover == 0) {
                    // Empty gap.
                    // Fill span if there is one and reset current span.
                    if (spanAlpha != 0) {
                        blender.CompositeSpan(spanX, spanEnd, d, spanAlpha);
                    }

                    spanX = edgeX;
                    spanEnd = nextEdgeX;
                    spanAlpha = alpha;
                } else {
                    // Non empty gap.
                    // Attempt to merge gap with current span.
                    const uint32 gapAlpha = ApplyFillRule(cover << 9);

                    // If alpha matches, extend current span.
                    if (spanAlpha == gapAlpha) {
                        if (alpha == gapAlpha) {
                            // Current pixel alpha matches as well.
                            spanEnd = nextEdgeX;
                        } else {
                            // Only gap alpha matches current span.
                            blender.CompositeSpan(spanX, edgeX, d, spanAlpha);

                            spanX = edgeX;
                            spanEnd = nextEdgeX;
                            spanAlpha = alpha;
                        }
                    } else {
                        if (spanAlpha != 0) {
                            blender.CompositeSpan(spanX, spanEnd, d, spanAlpha);
                        }

                        // Compose gap.
                        blender.CompositeSpan(spanEnd, edgeX, d, gapAlpha);

                        spanX = edgeX;
                        spanEnd = nextEdgeX;
                        spanAlpha = alpha;
                    }
                }
            }

            cover += coverAreaTable[tableIndex];
        }
    }

    if (spanAlpha != 0) {
        // Composite current span.
        blender.CompositeSpan(spanX, spanEnd, d, spanAlpha);
    }

    if (cover != 0 and spanEnd < rowLength) {
        // Composite anything that goes to the edge of destination image.
        const int32 alpha = ApplyFillRule(cover << 9);

        blender.CompositeSpan(spanEnd, rowLength, d, alpha);
    }
}


template <typename T>
FORCE_INLINE void Rasterizer<T>::RasterizeOneItem(const RasterizableItem *item,
    BitVector **bitVectorTable, int32 **coverAreaTable, const int columnCount,
    const ImageData &image)
{
    // A maximum number of horizontal tiles.
    const int horizontalCount = item->Rasterizable->Bounds.ColumnCount;

    ASSERT(horizontalCount <= columnCount);

    const int bitVectorsPerRow = BitVectorsForMaxBitCount(
        horizontalCount * T::TileW);

    // Erase bit vector table.
    for (int i = 0; i < T::TileH; i++) {
        memset(bitVectorTable[i], 0, SIZE_OF(BitVector) * bitVectorsPerRow);
    }

    item->Rasterizable->IterationFunction(item, bitVectorTable, coverAreaTable);

    // Pointer to backdrop.
    const int32 *coversStart = item->GetActualCovers();

    const int x = item->Rasterizable->Bounds.X * T::TileW;

    // Y position, measured in tiles.
    const int miny = item->Rasterizable->Bounds.Y + item->LocalRowIndex;

    // Y position, measure in pixels.
    const int py = miny * T::TileH;

    // Maximum y position, measured in pixels.
    const int maxpy = py + T::TileH;

    // Start row.
    uint8 *ptr = image.Data + (py * image.BytesPerRow);

    // Calculate maximum height. This can only get less than 8 when rendering
    // the last row of the image and image height is not multiple of row
    // height.
    const int hh = Min(maxpy, image.Height) - py;

    // Fill color.
    const uint32 color = item->Rasterizable->Geometry->Color;
    const FillRule rule = item->Rasterizable->Geometry->Rule;

    if (color >= 0xff000000) {
        if (rule == FillRule::NonZero) {
            for (int i = 0; i < hh; i++) {
                RenderOneLine<SpanBlenderOpaque, AreaToAlphaNonZero>(ptr,
                    bitVectorTable[i], bitVectorsPerRow, coverAreaTable[i], x,
                    image.Width, coversStart[i], color);

                ptr += image.BytesPerRow;
            }
        } else {
            for (int i = 0; i < hh; i++) {
                RenderOneLine<SpanBlenderOpaque, AreaToAlphaEvenOdd>(ptr,
                    bitVectorTable[i], bitVectorsPerRow, coverAreaTable[i], x,
                    image.Width, coversStart[i], color);

                ptr += image.BytesPerRow;
            }
        }
    } else {
        if (rule == FillRule::NonZero) {
            for (int i = 0; i < hh; i++) {
                RenderOneLine<SpanBlender, AreaToAlphaNonZero>(ptr,
                    bitVectorTable[i], bitVectorsPerRow, coverAreaTable[i], x,
                    image.Width, coversStart[i], color);

                ptr += image.BytesPerRow;
            }
        } else {
            for (int i = 0; i < hh; i++) {
                RenderOneLine<SpanBlender, AreaToAlphaEvenOdd>(ptr,
                    bitVectorTable[i], bitVectorsPerRow, coverAreaTable[i], x,
                    image.Width, coversStart[i], color);

                ptr += image.BytesPerRow;
            }
        }
    }
}


/**
 * Rasterize all items in one row.
 */
template <typename T>
FORCE_INLINE void Rasterizer<T>::RasterizeRow(
    const RowItemList<RasterizableItem> *rowList, ThreadMemory &memory,
    const ImageData &image)
{
    // How many columns can fit into image.
    const TileIndex columnCount = CalculateColumnCount<T>(image.Width);

    // Create bit vector arrays.
    const int bitVectorsPerRow = BitVectorsForMaxBitCount(
        columnCount * T::TileW);
    const int bitVectorCount = bitVectorsPerRow * T::TileH;

    BitVector *bitVectors = static_cast<BitVector *>(
        memory.TaskMalloc(SIZE_OF(BitVector) * bitVectorCount));

    // Create cover/area table.
    const int coverAreaIntsPerRow = columnCount * T::TileW * 2;
    const int coverAreaIntCount = coverAreaIntsPerRow * T::TileH;

    int32 *coverArea = static_cast<int32 *>(
        memory.TaskMalloc(SIZE_OF(int32) * coverAreaIntCount));

    // Setup row pointers for bit vectors and cover/area table.
    BitVector *bitVectorTable[T::TileH] ALIGNED(64);
    int32 *coverAreaTable[T::TileH] ALIGNED(64);

    for (int i = 0; i < T::TileH; i++) {
        bitVectorTable[i] = bitVectors;
        coverAreaTable[i] = coverArea;

        bitVectors += bitVectorsPerRow;
        coverArea += coverAreaIntsPerRow;
    }

    // Rasterize all items, from bottom to top that were added to this row.
    const typename RowItemList<RasterizableItem>::Block *b = rowList->First;

    while (b != nullptr) {
        const int count = b->Count;
        const RasterizableItem *itm = b->Items;
        const RasterizableItem *e = b->Items + count;

        while (itm < e) {
            RasterizeOneItem(itm++, bitVectorTable, coverAreaTable,
                columnCount, image);
        }

        b = b->Next;
    }
}
