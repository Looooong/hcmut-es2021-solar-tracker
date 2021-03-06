import express from 'express';
import expressStaticGzip from 'express-static-gzip';
import http, { IncomingMessage } from 'http';
import { Socket } from 'net';
import url from 'url';
import WebSocket from 'ws';

const app = express();
app.use(expressStaticGzip('public', {
    enableBrotli: true,
    orderPreference: ['br', 'gzip'],
}));
const server = http.createServer(app);
const wss = new WebSocket.Server({ noServer: true });

enum AppEvent {
    UpdateConfig = "UPDATE_CONFIG",
    UpdateState = "UPDATE_STATE",
}

enum ControlMode {
    Automatic = "AUTOMATIC",
    Manual = "MANUAL",
}

interface ControlConfig {
    controlMode?: ControlMode,
    manualOrientation?: Orientation,
}

interface SystemState {
    platformRotation: Quaternion,
    panelOrientation: Orientation,
    motorsRotation: Orientation,
    timestamp: number,
}

interface Orientation {
    azimuth: number,
    inclination: number,
}

interface Quaternion {
    w: number,
    x: number,
    y: number,
    z: number,
}

let config = {
    controlMode: ControlMode.Manual,
    manualOrientation: {
        azimuth: 0.0,
        inclination: 0.0,
    } as Orientation
} as ControlConfig;

function throwException(message: string): never {
    throw new Error(message);
}

server.on('upgrade', (request: IncomingMessage, socket: Socket, buffer: Buffer) => {
    const pathname = url.parse(request.url ?? throwException('Request does not have an URL!')).pathname;

    if (pathname == '/ws') {
        wss.handleUpgrade(request, socket, buffer, ws => wss.emit('connection', ws, request));
    } else {
        socket.destroy();
    }
});

wss.on('connection', (ws: WebSocket, request: IncomingMessage) => {
    const address = request.socket.remoteAddress ?? "unkown address";

    console.log(`A client from ${address} connected.`);

    ws.on('close', (code, reason) => console.log(`A client from ${address} disconnected with code ${code}, reason: ${reason}.`));

    ws.on('message', data => {
        try {
            let request: {
                event: AppEvent,
                payload?: {} | []
            } = JSON.parse(data as string);

            wss.emit(request.event, ws, request.payload);
        } catch (e: any) {
            console.log(`Error parsing JSON: ${e}\n---------\nJSON string:\n\t${data}`);
        }
    });

    ws.on(AppEvent.UpdateConfig, () => {
        ws.send(JSON.stringify({
            event: AppEvent.UpdateConfig,
            payload: config,
        }));
    });

    ws.on(AppEvent.UpdateState, (new_state: SystemState) => {
        ws.send(JSON.stringify({
            event: AppEvent.UpdateState,
            payload: new_state,
        }))
    });

    ws.emit(AppEvent.UpdateConfig);
});

wss.on(AppEvent.UpdateConfig, (ws: WebSocket, new_config: ControlConfig) => {
    config.controlMode = new_config.controlMode ?? config.controlMode;
    config.manualOrientation = new_config.manualOrientation ?? config.manualOrientation;

    for (let client of wss.clients) {
        if (new_config && client == ws) continue;

        client.emit(AppEvent.UpdateConfig);
    }
});

wss.on(AppEvent.UpdateState, (ws: WebSocket, new_state: SystemState) => {
    for (let client of wss.clients) {
        if (client == ws) continue;

        client.emit(AppEvent.UpdateState, new_state);
    }
});

const port = parseInt(process.env['PORT'] ?? '8080');

server.listen(port, () => console.log(`Server is listening to port ${port}`));
