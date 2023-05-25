
var Module = null;
var priv_PreviousGestureScale = 1.0;

const priv_Path = "/";
const priv_RenderFrame_FnName = 'RenderFrame';
const priv_TranslateCanvas_FnName = 'TranslateCanvas';
const priv_ScaleCanvas_FnName = 'ScaleCanvas';
const priv_InstallVectorImage_FnName = 'InstallVectorImage';


function priv_RenderFrame() {
    Module.ccall(priv_RenderFrame_FnName);
}


function priv_TranslateCanvas(x, y) {
    Module.ccall(priv_TranslateCanvas_FnName, '', [ 'number', 'number' ],
        [ x, y ]);
}


function priv_ScaleCanvas(delta, x, y) {
    Module.ccall(priv_ScaleCanvas_FnName, '', [ 'number', 'number', 'number' ],
        [ delta, x, y ]);
}


function priv_InstallVectorImage(ptr, size) {
    Module.ccall(priv_InstallVectorImage_FnName, '', [ 'number', 'number', ],
        [ ptr, size ]);
}


function priv_CanvasMouseDown(event) {
    event.preventDefault();
}


function priv_CanvasMouseMove(event) {
    if (event.buttons == 1) {
        priv_TranslateCanvas(event.movementX * window.devicePixelRatio,
            event.movementY * window.devicePixelRatio);
    }

    event.preventDefault();
}


function priv_CanvasMouseUp(event) {
    event.preventDefault();
}


function priv_CanvasMouseWheel(event) {
    // Apparently, this hack (control key being set as modifier) is a standard
    // kind of hack supported by at least Chrome and Firefox as a way to
    // detect pinch gesture.
    if (event.ctrlKey) {
        const delta = -event.deltaY * 0.006;

        priv_ScaleCanvas(delta, event.clientX * window.devicePixelRatio,
            event.clientY * window.devicePixelRatio);
    } else {
        priv_TranslateCanvas(-event.deltaX, -event.deltaY);
    }

    event.preventDefault();
}


function priv_PointerMove(event) {
    event.preventDefault();
}


function priv_TouchMove(event) {
    event.preventDefault();
}


function priv_TouchEnd(event) {
    event.preventDefault();
}


function priv_AppleGestureStart(event) {
    priv_PreviousGestureScale = 1.0;
}


function priv_AppleGestureChange(event) {
    const delta = (event.scale - priv_PreviousGestureScale) * 0.12;

    priv_ScaleCanvas(delta, event.clientX * window.devicePixelRatio,
        event.clientY * window.devicePixelRatio);

    priv_PreviousGestureScale = event.scale;
}


function priv_AppleGestureEnd(event) {
    priv_PreviousGestureScale = 1.0;
}


function priv_DownloadAndInstallVectorImage(path) {
    var d = document.getElementById('modal-overlay');

    var xhr = new XMLHttpRequest();
    xhr.open('GET', priv_Path + path, true);
    xhr.responseType = 'arraybuffer';

    xhr.onload = function() {
        if (xhr.status === 200) {
            const bytes = new Uint8Array(xhr.response);
            const count = bytes.length * bytes.BYTES_PER_ELEMENT;
            const allocation = Module._malloc(count);

            Module.HEAPU8.set(bytes, allocation);

            priv_InstallVectorImage(allocation, count);

            Module._free(allocation);

            d.classList.remove('modal-overlay-open');

            priv_RenderFrame();
        } else {
            // Error.
            d.classList.remove('modal-overlay-open');
        }
    };

    xhr.onprogress = function(event) {
        if (event.lengthComputable) {
            const percentComplete = (event.loaded / event.total) * 100;

            const d = document.getElementById('progress');

            d.style.width = Math.max(percentComplete, 5) + '%';
        }
    };

    xhr.send();
}


function priv_Download(path) {
    var d = document.getElementById('modal-overlay');
    var m = document.getElementById('image-menu');
    var e = document.getElementById('image-menu-content');
    var b = document.getElementById('progress');

    m.classList.remove('image-menu-open');
    e.classList.remove('image-menu-content-open');
    d.classList.add('modal-overlay-open');
    b.style.width = 0;

    priv_DownloadAndInstallVectorImage(path);
}


function priv_onRuntimeInitialized() {
    const canvas = document.getElementById("canvas");

    canvas.addEventListener('mousedown', priv_CanvasMouseDown);
    canvas.addEventListener('mousemove', priv_CanvasMouseMove);
    canvas.addEventListener('mouseup', priv_CanvasMouseDown);
    canvas.addEventListener('wheel', priv_CanvasMouseWheel);
    canvas.addEventListener('pointermove', priv_PointerMove);
    canvas.addEventListener('touchmove', priv_TouchMove);
    canvas.addEventListener('touchend', priv_TouchEnd);
    canvas.addEventListener('gesturestart', priv_AppleGestureStart);
    canvas.addEventListener('gesturechange', priv_AppleGestureChange);
    canvas.addEventListener('gestureend', priv_AppleGestureEnd);

    const observer = new ResizeObserver(resizeTheCanvasToDisplaySize)

    observer.observe(canvas);

    function resizeTheCanvasToDisplaySize(entries) {
        if (entries.length > 0) {
            const entry = entries[0];

            let width = 0;
            let height = 0;

            if (entry.devicePixelContentBoxSize) {
                width = entry.devicePixelContentBoxSize[0].inlineSize;
                height = entry.devicePixelContentBoxSize[0].blockSize;
            } else if (entry.contentBoxSize) {
                width = Math.round(entry.contentBoxSize[0].inlineSize * devicePixelRatio);
                height = Math.round(entry.contentBoxSize[0].blockSize * devicePixelRatio);
            }

            if (canvas.width != width || canvas.height != height) {
                canvas.width = width;
                canvas.height = height;

                priv_RenderFrame();
            }
        }
    }

    priv_Download("instructions.vectorimage");
}


function priv_ToggleImageMenu() {
    var m = document.getElementById('image-menu');
    var e = document.getElementById('image-menu-content');

    if (e.classList.contains('image-menu-content-open')) {
        m.classList.remove('image-menu-open');
        e.classList.remove('image-menu-content-open');
    } else {
        e.style.top = m.offsetTop + m.offsetHeight - 8 + 'px';

        m.classList.add('image-menu-open');
        e.classList.add('image-menu-content-open');
    }
}


function processFatalFailure() {
    var e = document.getElementById('error-overlay');

    e.classList.add('error-overlay-open');
}


function installBinarySequence(pathArray) {
    if (pathArray.length < 1) {
        // There is nothing available to try anymore.
        processFatalFailure();
        return;
    }

    let path = pathArray[0];

    let e = document.createElement("script");

    e.addEventListener("load", () => {
        // Wrap module creator invocation in try-catch block to catch any
        // exceptions thrown when initializing module. For example, if
        // SharedArrayBuffer is not available, createModule() will fail
        // without invoking catch() function.
        //
        // But if exception is thrown instead of invoking catch() function, it
        // will be considered as fatal failure.
        try {
            createModule().then(m => {
                Module = m;
                priv_onRuntimeInitialized();
            }).catch(() => {
                // Unlink failed script element from document.
                e.remove();

                // Remove the first element and try again.
                installBinary(pathArray.slice(1));
            });
        } catch (error) {
            processFatalFailure();
        }
    });

    e.addEventListener("error", (ex) => {
        alert("Error loading WebAssembly module");
    });

    e.setAttribute("src", priv_Path + path);
    e.setAttribute("type", "text/javascript");
    e.setAttribute("async", true);

    document.body.appendChild(e);
}


function is_iOS() {
    if (navigator.userAgent.match(/iPad|iPhone|iPod/)) {
        return true;
    }

    if (navigator.userAgent.match(/Mac/) && navigator.maxTouchPoints && navigator.maxTouchPoints > 2) {
        return true;
    }

    return false;
}


function installApplication() {
    if (is_iOS()) {
        // On iOS, there is not a lot of options.
        installBinarySequence([ "index-2.js" ]);
    } else {
        installBinarySequence([ "index-0.js", "index-1.js", "index-2.js" ]);
    }
}


installApplication();
