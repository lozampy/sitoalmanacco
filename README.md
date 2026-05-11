# sitoalmanacco
same tingh as almanaccomagico but in html
finished in like 2 days or 18 hours

## C++ Backend

A lightweight C++ HTTP server is included to serve the static files and provide API endpoints.

### Building

```bash
make
```

### Running

```bash
./server
```

The server runs on `http://localhost:8080`

### Features

- Serves static files (HTML, CSS, JS, JSON, images)
- API endpoint: `/api/hello` - Returns a JSON response
- CORS enabled for cross-origin requests
- Directory traversal protection

### Stopping

Press `Ctrl+C` to stop the server.
