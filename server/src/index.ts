import express from 'express';
import http, { IncomingMessage } from 'http';
import { Socket } from 'net';
import url from 'url';
import WebSocket from 'ws';

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ noServer: true });

function throwException(message: string): never {
    throw new Error(message);
}

server.on('upgrade', (request: IncomingMessage, socket: Socket, buffer: Buffer) => {
    const pathname = url.parse(request.url ?? throwException('Request does not have an URL!')).pathname;

    if (pathname == '/ws') {
        wss.handleUpgrade(request, socket, buffer, ws => wss.emit('connection', ws, request));
    }
});

wss.on('connection', (ws: WebSocket, request: IncomingMessage) => {
    console.log(`A client connected from ${request.socket.remoteAddress ?? "unkown address"}.`);

    ws.on('message', message => console.log(`Received: ${message}`));
});

const interval = setInterval(() => {
    wss.clients.forEach(ws => {
        ws.send('Hello from server!');
    });
}, 2000);

wss.on('close', () => clearInterval(interval));

const port = parseInt(process.env['PORT'] ?? '8080');

console.log(`Server is listening to port ${port}`);
server.listen(port);
