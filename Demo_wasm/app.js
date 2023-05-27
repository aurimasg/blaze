
const priv_Path = "/";
const priv_RenderFrame_FnName = 'RenderFrame';
const priv_TranslateCanvas_FnName = 'TranslateCanvas';
const priv_ScaleCanvas_FnName = 'ScaleCanvas';
const priv_InstallVectorImage_FnName = 'InstallVectorImage';


function is_iOS() {
    if (navigator.userAgent.match(/iPad|iPhone|iPod/)) {
        return true;
    }

    if (navigator.userAgent.match(/Mac/) && navigator.maxTouchPoints && navigator.maxTouchPoints > 2) {
        return true;
    }

    return false;
}


function wheelEventDelta(event) {
    if (navigator.userAgent.match(/chrome|chromium|crios/i)) {
        return event.deltaY * 0.003;
    }

    return event.deltaY * 0.0045;
}


function lerp(a, b, t) {
    return a + t * (b - a);
}


function touchByTouchIdentifier(touchList, identifier) {
    for (let i = 0; i < touchList.length; i++) {
        if (touchList.item(i).identifier === identifier) {
            return touchList.item(i);
        }
    }

    return null;
}


/**
 * Represents a very basic set of properties for identifying a single touch
 * with position of where it started and where it went.
 */
class BlazeTouch {
    x = 0;
    y = 0;
    toX = 0;
    toY = 0;
    prevX = 0;
    prevY = 0;
    identifier = 0;

    constructor(x, y, identifier) {
        this.x = x;
        this.y = y;
        this.toX = x;
        this.toY = y;
        this.prevX = x;
        this.prevY = y;
        this.identifier = identifier;
    }


    /**
     * Returns travel distance of this touch from its starting position to
     * current position. Always returns positive number or 0.
     */
    travelDistance() {
        return Math.hypot(this.toX - this.x, this.toY - this.Y);
    }


    /**
     * Returns travel distance of this touch from its previous position to
     * current position. Always returns positive number or 0.
     */
    travelDistanceFromPrevious() {
        return Math.hypot(this.toX - this.prevX, this.toY - this.prevY);
    }
}


class FloatPoint {
    x = 0;
    y = 0;

    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
}


class BlazeImageMenu {
    button = null;
    menu = null;

    constructor() {
        this.button = document.getElementById('image-menu');

        this.button.addEventListener('click', (event) => {
            if (this.menu.classList.contains('image-menu-content-open')) {
                this.button.classList.remove('image-menu-open');
                this.menu.classList.remove('image-menu-content-open');
            } else {
                this.menu.style.top = this.button.offsetTop + this.button.offsetHeight - 8 + 'px';
                this.button.classList.add('image-menu-open');
                this.menu.classList.add('image-menu-content-open');
            }

            event.preventDefault();
        });

        this.menu = document.getElementById('image-menu-content');

        document.getElementById('open-tiger').addEventListener('click', (event) => {
            canvas.download('tiger.vectorimage');
            event.preventDefault();
        });

        document.getElementById('open-paris-30k').addEventListener('click', (event) => {
            canvas.download('paris-30k.vectorimage');
            event.preventDefault();
        });

        document.getElementById('open-paragraphs').addEventListener('click', (event) => {
            canvas.download('paragraphs.vectorimage');
            event.preventDefault();
        });

        document.getElementById('open-boston').addEventListener('click', (event) => {
            canvas.download('boston.vectorimage');
            event.preventDefault();
        });

        document.getElementById('open-periodic-table').addEventListener('click', (event) => {
            canvas.download('periodic-table.vectorimage');
            event.preventDefault();
        });
    }
}


let menu = new BlazeImageMenu();


class BlazeCanvas {
    module = null;
    canvas = null;
    touches = [];
    touchCenter = new FloatPoint(0, 0);

    constructor() {
        if (is_iOS()) {
            // On iOS, there is not a lot of options.
            this.installBinarySequence([ "index-2.js" ]);
        } else {
            this.installBinarySequence([ "index-0.js", "index-1.js", "index-2.js" ]);
        }
    }

    installBinarySequence(pathArray) {
        if (pathArray.length < 1) {
            // There is nothing available to try anymore.
            this.processFatalFailure();
            return;
        }

        let path = pathArray[0];

        let element = document.createElement("script");

        element.addEventListener("load", () => {
            // Wrap module creator invocation in try-catch block to catch any
            // exceptions thrown when initializing module. For example, if
            // SharedArrayBuffer is not available, createModule() will fail
            // without invoking catch() function.
            //
            // But if exception is thrown instead of invoking catch() function, it
            // will be considered as fatal failure.
            try {
                createModule().then(m => {
                    this.module = m;
                    this.init();
                }).catch(() => {
                    // Unlink failed script element from document.
                    element.remove();

                    // Remove the first element and try again.
                    this.installBinarySequence(pathArray.slice(1));
                });
            } catch (error) {
                this.processFatalFailure();
            }
        });

        element.addEventListener("error", (ex) => {
            alert("Error loading WebAssembly module");
        });

        element.setAttribute("src", priv_Path + path);
        element.setAttribute("type", "text/javascript");
        element.setAttribute("async", true);

        document.body.appendChild(element);
    }

    processFatalFailure() {
        var element = document.getElementById('error-overlay');

        element.classList.add('error-overlay-open');
    }


    /**
     * Returns existing touch with a given id or null if there is no such
     * touch.
     */
    findExistingTouchWithId(identifier) {
        for (let i = 0; i < this.touches.length; i++) {
            if (this.touches[i].identifier === identifier) {
                return this.touches[i];
            }
        }

        return null;
    }


    /**
     * Finds drag distance between center of start multi-touch gesture and
     * center of current multi-touch gesture.
     */
    calculateTouchCenterDragDistance() {
        if (this.touches.length < 2) {
            return new FloatPoint(0, 0);
        }

        let ax = 0;
        let ay = 0;
        let bx = 0;
        let by = 0;

        for (let i = 0; i < this.touches.length; i++) {
            ax += this.touches[i].prevX;
            ay += this.touches[i].prevY;
            bx += this.touches[i].toX;
            by += this.touches[i].toY;
        }

        const ax2 = ax / this.touches.length;
        const ay2 = ay / this.touches.length;
        const bx2 = bx / this.touches.length;
        const by2 = by / this.touches.length;

        return new FloatPoint(ax2 - bx2, ay2 - by2);
    }


    /**
     * Takes touch event as input and updates active touches. Returns true if
     * at least one touch was either dropped or added.
     */
    updateTouchesForEvent(event) {
        let num0 = this.touches.length;

        // Add or update active touches.
        for (let i = 0; i < event.touches.length; i++) {
            const identifier = event.touches[i].identifier;
            const cx = event.touches[i].clientX;
            const cy = event.touches[i].clientY;

            let touch = this.findExistingTouchWithId(identifier);

            if (touch != null) {
                // Update existing touch.
                touch.prevX = touch.toX;
                touch.prevY = touch.toY;
                touch.toX = cx;
                touch.toY = cy;
            } else {
                // New touch.
                const nt = new BlazeTouch(cx, cy, identifier);

                this.touches.push(nt);
            }
        }

        const num1 = this.touches.length;

        // Remove dropped touches.
        if (num1 > 0) {
            for (let i = num1 - 1; i >= 0; i--) {
                const activeTouch = touchByTouchIdentifier(event.touches,
                    this.touches[i].identifier);

                if (activeTouch == null) {
                    // There is an active touch which is not in event touch
                    // array anymore.
                    this.touches.splice(i, 1);
                }
            }
        }

        // Return true if active touch count did not change at any stage.
        const change = num0 != num1 || num1 != this.touches.length;

        if (change) {
            let ax = 0;
            let ay = 0;

            for (let i = 0; i < this.touches.length; i++) {
                ax += this.touches[i].x;
                ay += this.touches[i].y;
            }

            this.touchCenter = new FloatPoint(
                ax / this.touches.length,
                ay / this.touches.length);
        }

        return change;
    }


    init() {
        let canvas = document.getElementById('canvas');

        this.canvas = canvas;

        const canvasMouseDown = (ev) => {
            ev.preventDefault();
        }

        const canvasMouseMove = (ev) => {
            if (ev.buttons == 1) {
                this.translateCanvas(ev.movementX * window.devicePixelRatio,
                    ev.movementY * window.devicePixelRatio);
            }

            ev.preventDefault();
        }

        const canvasMouseWheel = (ev) => {
            // Apparently, this hack (control key being set as modifier) is a
            // standard hack supported by at least Chrome and Firefox as a way
            // to detect pinch gesture.
            if (ev.ctrlKey) {
                const delta = -wheelEventDelta(ev) * window.devicePixelRatio;
                const cx = ev.clientX * window.devicePixelRatio;
                const cy = ev.clientY * window.devicePixelRatio;

                this.scaleCanvas(1.0 + delta, cx, cy);
            } else {
                this.translateCanvas(-ev.deltaX, -ev.deltaY);
            }

            ev.preventDefault();
        }

        const canvasPointerMove = (ev) => {
            ev.preventDefault();
        }

        const canvasTouchStart = (ev) => {
            this.updateTouchesForEvent(ev);
            ev.preventDefault();
        }

        const canvasTouchMove = (ev) => {
            const change = this.updateTouchesForEvent(ev);

            if (this.touches.length == 2 && !change) {
                const touchA = this.touches[0];
                const touchB = this.touches[1];

                // Distance between previous touch positions.
                const distanceBetweenPrevious =
                    Math.hypot(touchB.prevX - touchA.prevX, touchB.prevY - touchA.prevY);

                // Distance between current touch positions.
                const distanceBetweenCurrent =
                    Math.hypot(touchB.toX - touchA.toX, touchB.toY - touchA.toY);

                const r = window.devicePixelRatio;
                const distanceDelta = Math.abs(distanceBetweenPrevious - distanceBetweenCurrent);

                if (distanceDelta > Number.EPSILON) {
                    if (distanceBetweenPrevious > Number.EPSILON) {
                        const scaleDelta = distanceBetweenCurrent / distanceBetweenPrevious;

                        this.scaleCanvas(scaleDelta, this.touchCenter.x * r,
                            this.touchCenter.y * r);
                    }
                }

                const min = this.calculateTouchCenterDragDistance();

                this.translateCanvas(-min.x * r, -min.y * r);

                this.touchCenter.x -= min.x;
                this.touchCenter.y -= min.y;
            }

            ev.preventDefault();
        }

        const canvasTouchEnd = (ev) => {
            this.updateTouchesForEvent(ev);
            ev.preventDefault();
        }

        canvas.addEventListener('mousedown', canvasMouseDown);
        canvas.addEventListener('mousemove', canvasMouseMove);
        canvas.addEventListener('mouseup', canvasMouseDown);
        canvas.addEventListener('wheel', canvasMouseWheel);
        canvas.addEventListener('pointermove', canvasPointerMove);
        canvas.addEventListener('touchstart', canvasTouchStart);
        canvas.addEventListener('touchmove', canvasTouchMove);
        canvas.addEventListener('touchend', canvasTouchEnd);

        let observer = new ResizeObserver((entries) => {
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

                if (this.canvas.width != width || this.canvas.height != height) {
                    this.canvas.width = width;
                    this.canvas.height = height;
                    this.renderFrame();
                }
            }
        });

        observer.observe(canvas);

        this.download('instructions.vectorimage');
    }

    renderFrame() {
        this.module.ccall(priv_RenderFrame_FnName);
    }

    translateCanvas(x, y) {
        this.module.ccall(priv_TranslateCanvas_FnName, '', [ 'number', 'number' ],
            [ x, y ]);
    }

    scaleCanvas(delta, x, y) {
        this.module.ccall(priv_ScaleCanvas_FnName, '', [ 'number', 'number', 'number' ],
            [ delta, x, y ]);
    }

    installVectorImage(ptr, size) {
        this.module.ccall(priv_InstallVectorImage_FnName, '', [ 'number', 'number', ],
            [ ptr, size ]);
    }

    downloadAndInstallVectorImage(path) {
        var d = document.getElementById('modal-overlay');

        var xhr = new XMLHttpRequest();
        xhr.open('GET', priv_Path + path, true);
        xhr.responseType = 'arraybuffer';

        xhr.onload = () => {
            if (xhr.status === 200) {
                const bytes = new Uint8Array(xhr.response);
                const count = bytes.length * bytes.BYTES_PER_ELEMENT;
                const allocation = this.module._malloc(count);

                this.module.HEAPU8.set(bytes, allocation);

                this.installVectorImage(allocation, count);

                this.module._free(allocation);

                d.classList.remove('modal-overlay-open');

                this.renderFrame();
            } else {
                // Error.
                d.classList.remove('modal-overlay-open');
            }
        };

        xhr.onprogress = (event) => {
            if (event.lengthComputable) {
                const percentComplete = (event.loaded / event.total) * 100;

                const d = document.getElementById('progress');

                d.style.width = Math.max(percentComplete, 5) + '%';
            }
        };

        xhr.send();
    }

    download(path) {
        let d = document.getElementById('modal-overlay');
        let m = document.getElementById('image-menu');
        let e = document.getElementById('image-menu-content');
        let b = document.getElementById('progress');

        m.classList.remove('image-menu-open');
        e.classList.remove('image-menu-content-open');
        d.classList.add('modal-overlay-open');
        b.style.width = 0;

        this.downloadAndInstallVectorImage(path);
    }
}

let canvas = new BlazeCanvas();
