
from http.server import HTTPServer, SimpleHTTPRequestHandler

class HTTPRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_k_headers()
        SimpleHTTPRequestHandler.end_headers(self)

    def send_k_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")

sr = HTTPServer(("", 80), HTTPRequestHandler)

sr.serve_forever()
