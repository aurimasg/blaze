
#import <Blaze/Blaze.h>
#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <simd/simd.h>
#import "AppView.h"


struct ViewData final {
    Matrix CoordinateSystemMatrix;
    FloatPoint Translation;
    double Scale = 1;

    void SetupCoordinateSystem(const int width, const int height, const VectorImage &image);

    Matrix GetMatrix() const;
};


void ViewData::SetupCoordinateSystem(const int width, const int height,
    const VectorImage &image)
{
    const IntRect bounds = image.GetBounds();

    const double mx = double(bounds.MaxX - bounds.MinX) / 2.0;
    const double my = double(bounds.MaxY - bounds.MinY) / 2.0;

    CoordinateSystemMatrix = Matrix::CreateTranslation(
        (double(width) / 2.0) - mx,
        (double(height) / 2.0) - my);
}


Matrix ViewData::GetMatrix() const
{
    Matrix m = Matrix::CreateScale(Scale, Scale);

    m.PreTranslate(Translation.X, Translation.Y);

    m.PreMultiply(CoordinateSystemMatrix);

    return m;
}


@implementation AppView {
    CVDisplayLinkRef mDisplayLink;
    CAMetalLayer *mMetalLayer;

    DestinationImage<TileDescriptor_8x16> mImage;

    id<MTLTexture> mTexture;
    id<MTLRenderPipelineState>mPS;
    id<MTLCommandQueue> mQueue;

    VectorImage mVectorImage;

    dispatch_semaphore_t mSemaphore;

    ViewData mViewData;

    NSLock *mViewDataLock;
}


- (instancetype)initWithFrame:(NSRect)bounds {
    self = [super initWithFrame:bounds];

    mSemaphore = dispatch_semaphore_create(1);

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    mMetalLayer = CAMetalLayer.layer;
    mMetalLayer.framebufferOnly = YES;
    mMetalLayer.device = device;
    mMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    mMetalLayer.displaySyncEnabled = YES;
    mMetalLayer.presentsWithTransaction = NO;

    CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    mMetalLayer.colorspace = colorspace;

    CGColorSpaceRelease(colorspace);

    const NSSize size = [self convertSizeToBacking:self.bounds.size];

    mMetalLayer.drawableSize = CGSizeMake(size.width, size.height);

    id<MTLLibrary> library = device.newDefaultLibrary;

    MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
    d.vertexFunction = [library newFunctionWithName:@"VertexFn"];
    d.fragmentFunction = [library newFunctionWithName:@"FragmentFn"];
    d.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    d.colorAttachments[0].blendingEnabled = YES;
    d.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    d.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    d.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    d.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    d.colorAttachments[0].destinationRGBBlendFactor =
        MTLBlendFactorOneMinusSourceAlpha;
    d.colorAttachments[0].destinationAlphaBlendFactor =
        MTLBlendFactorOneMinusSourceAlpha;

    mPS = [device newRenderPipelineStateWithDescriptor:d error:nil];

    mQueue = [device newCommandQueue];

    mViewDataLock = [[NSLock alloc] init];

    self.layer = mMetalLayer;

    [self loadImageAtPath:[NSBundle.mainBundle pathForResource:@"Instructions" ofType:@"vectorimage"]];

    return self;
}


- (void)renderFrame {
    const int tw = int(mMetalLayer.drawableSize.width);
    const int th = int(mMetalLayer.drawableSize.height);

    if (tw < 1 or th < 1) {
        return;
    }

    const IntSize imageSize = mImage.UpdateSize(IntSize {
        tw, th
    });

    [mViewDataLock lock];

    const Matrix matrix = mViewData.GetMatrix();

    mImage.ClearImage();
    mImage.DrawImage(mVectorImage, matrix);

    [mViewDataLock unlock];

    // Update texture.
    if (mTexture.width < imageSize.Width or mTexture.height < imageSize.Height) {
        MTLTextureDescriptor *d = [MTLTextureDescriptor new];
        d.width = imageSize.Width;
        d.height = imageSize.Height;
        d.pixelFormat = MTLPixelFormatRGBA8Unorm;
        d.usage = MTLTextureUsageShaderRead;

        mTexture = [mMetalLayer.device newTextureWithDescriptor:d];
    }

    // Upload rasterized image.
    [mTexture replaceRegion:MTLRegionMake2D(0, 0, imageSize.Width, imageSize.Height)
        mipmapLevel:0
        withBytes:mImage.GetImageData()
        bytesPerRow:mImage.GetBytesPerRow()];

    // Draw texture.
    const simd::float1 minx = static_cast<simd::float1>(0);
    const simd::float1 miny = static_cast<simd::float1>(0);
    const simd::float1 maxx = static_cast<simd::float1>(minx + mTexture.width);
    const simd::float1 maxy = static_cast<simd::float1>(miny + mTexture.height);

    const simd::float2 vertices[] = {
        { minx, miny }, { 0.0f, 0.0f },
        { maxx, miny }, { 1.0f, 0.0f },
        { minx, maxy }, { 0.0f, 1.0f },

        { minx, maxy }, { 0.0f, 1.0f },
        { maxx, miny }, { 1.0f, 0.0f },
        { maxx, maxy }, { 1.0f, 1.0f }
    };

    simd::int2 renderData = {
        tw, th
    };

    // Wait for the previous frame to finish.
    dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);

    id<CAMetalDrawable> drawable = [mMetalLayer nextDrawable];

    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1, 1, 1, 1);

    id<MTLCommandBuffer> commandBuffer = mQueue.commandBuffer;

    id<MTLRenderCommandEncoder> encoder = [commandBuffer
        renderCommandEncoderWithDescriptor:renderPassDescriptor];

    [encoder setRenderPipelineState:mPS];
    [encoder setLabel:@"Render texture"];
    [encoder setVertexBytes:&vertices length:SIZE_OF(vertices) atIndex:0];
    [encoder setVertexBytes:&renderData length:SIZE_OF(renderData) atIndex:1];
    [encoder setFragmentTexture:mTexture atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0
        vertexCount:6];
    [encoder endEncoding];

    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull) {
        dispatch_semaphore_signal(self->mSemaphore);
    }];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}


- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];

    const NSSize size = [self convertSizeToBacking:newSize];

    [mMetalLayer setDrawableSize:CGSizeMake(size.width, size.height)];

    [mViewDataLock lock];

    mViewData.SetupCoordinateSystem(int(size.width), int(size.height),
        mVectorImage);

    [mViewDataLock unlock];
}


- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
    if (newWindow == nil) {
        if (mDisplayLink != nullptr) {
            CVDisplayLinkStop(mDisplayLink);
            CVDisplayLinkRelease(mDisplayLink);

            mDisplayLink = nullptr;
        }
    } else {
        if (mDisplayLink == nullptr) {
            CVDisplayLinkCreateWithActiveCGDisplays(&mDisplayLink);

            CVDisplayLinkSetOutputHandler(mDisplayLink, ^CVReturn(CVDisplayLinkRef  _Nonnull displayLink, const CVTimeStamp * _Nonnull inNow, const CVTimeStamp * _Nonnull inOutputTime, CVOptionFlags flagsIn, CVOptionFlags * _Nonnull flagsOut) {
                [self renderFrame];

                return kCVReturnSuccess;
            });

            CVDisplayLinkStart(mDisplayLink);
        }
    }
}


- (BOOL)isFlipped {
    return YES;
}


- (void)mouseDown:(NSEvent *)event {
    [mViewDataLock lock];

    const double tx = mViewData.Translation.X;
    const double ty = mViewData.Translation.Y;

    [mViewDataLock unlock];

    const NSPoint p = [self convertPoint:event.locationInWindow fromView:nil];

    while (NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny
       untilDate:NSDate.distantFuture
          inMode:NSEventTrackingRunLoopMode
         dequeue:YES])
    {
        if (e.type == NSEventTypeLeftMouseUp) {
            break;
        } else if (e.type == NSEventTypeLeftMouseDragged) {
            const NSPoint to = [self convertPoint:e.locationInWindow
                fromView:nil];
            const double dx = round((to.x - p.x) * self.window.backingScaleFactor);
            const double dy = round((to.y - p.y) * self.window.backingScaleFactor);

            [mViewDataLock lock];

            mViewData.Translation.X = tx + dx;
            mViewData.Translation.Y = ty + dy;

            [mViewDataLock unlock];
        }
    }
}


- (void)doMagnify:(double)delta viewPoint:(NSPoint)viewPoint {
    [mViewDataLock lock];

    const double fraction = mViewData.Scale;

    [mViewDataLock unlock];

    const double magnification = fraction * delta;
    const double newZoom = MIN(MAX(fraction + magnification, 0.01), 100000.0);

    if (fraction != newZoom) {
        const double dd = (newZoom - fraction) / fraction;

        [mViewDataLock lock];

        const FloatPoint pn = mViewData.CoordinateSystemMatrix.Inverse().Map(
            viewPoint.x, viewPoint.y);

        mViewData.Translation.X += (mViewData.Translation.X - pn.X) * dd;
        mViewData.Translation.Y += (mViewData.Translation.Y - pn.Y) * dd;
        mViewData.Scale = newZoom;

        [mViewDataLock unlock];
    }
}


- (void)magnifyWithEvent:(NSEvent *)theEvent {
    const NSPoint pt = [self convertPoint:theEvent.locationInWindow fromView:nil];
    const NSPoint p = NSMakePoint(pt.x * self.window.backingScaleFactor, pt.y * self.window.backingScaleFactor);

    [self doMagnify:theEvent.magnification viewPoint:p];
}


- (void)scrollWheel:(NSEvent *)event {
    if (event.modifierFlags & NSEventModifierFlagOption) {
        const NSPoint pt = [self convertPoint:event.locationInWindow fromView:nil];
        const NSPoint p = NSMakePoint(pt.x * self.window.backingScaleFactor, pt.y * self.window.backingScaleFactor);

        if (event.hasPreciseScrollingDeltas) {
            [self doMagnify:event.scrollingDeltaY * 0.001 viewPoint:p];
        } else {
            [self doMagnify:event.deltaY * 0.001 viewPoint:p];
        }
    } else {
        [mViewDataLock lock];

        if (event.hasPreciseScrollingDeltas) {
            mViewData.Translation.X += event.scrollingDeltaX;
            mViewData.Translation.Y += event.scrollingDeltaY;
        } else {
            mViewData.Translation.X += (event.deltaX * 6.0);
            mViewData.Translation.Y += (event.deltaY * 6.0);
        }

        [mViewDataLock unlock];
    }
}


- (void)loadImageAtPath:(NSString *)path {
    NSData *d = [NSData dataWithContentsOfFile:path];

    if (d != nil) {
        [mViewDataLock lock];

        mVectorImage.Parse((const uint8 *)d.bytes, (uint64)d.length);

        mViewData.SetupCoordinateSystem(int(mMetalLayer.drawableSize.width),
            int(mMetalLayer.drawableSize.height), mVectorImage);
        mViewData.Scale = 1;
        mViewData.Translation.X = 0;
        mViewData.Translation.Y = 0;

        [mViewDataLock unlock];
    }
}


@end
