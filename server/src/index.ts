import express from 'express';
import http, { IncomingMessage } from 'http';
import { Socket } from 'net';
import url from 'url';
import WebSocket from 'ws';

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ noServer: true });

enum AppEvent {
    UpdateConfig = "UPDATE_CONFIG",
    UpdateStates = "UPDATE_STATES",
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
    timestamp: number,
    solarPanelVoltage: number,
}

interface Orientation {
    azimuth: number,
    inclination: number,
}

let config = {
    controlMode: ControlMode.Automatic,
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
        console.log(`Received:\n${data}`);

        let request: {
            event: AppEvent,
            payload?: {} | []
        } = JSON.parse(data as string);

        wss.emit(request.event, ws, request.payload);
    });

    ws.on(AppEvent.UpdateConfig, () => {
        ws.send(JSON.stringify({
            event: AppEvent.UpdateConfig,
            payload: config,
        }));
    });

    ws.on(AppEvent.UpdateStates, (new_states: SystemState[]) => {
        ws.send(JSON.stringify({
            event: AppEvent.UpdateStates,
            payload: new_states,
        }))
    });

    ws.emit(AppEvent.UpdateConfig);
});

wss.on(AppEvent.UpdateConfig, (ws: WebSocket, new_config: ControlConfig) => {
    config.controlMode = new_config.controlMode ?? config.controlMode;
    config.manualOrientation = new_config.manualOrientation ?? config.manualOrientation;

    for (let client of wss.clients) {
        if (client == ws) continue;

        client.emit(AppEvent.UpdateConfig);
    }
});

wss.on(AppEvent.UpdateStates, (ws: WebSocket, new_states: SystemState[]) => {
    for (let client of wss.clients) {
        if (client == ws) continue;

        client.emit(AppEvent.UpdateStates, new_states);
    }
});

const port = parseInt(process.env['PORT'] ?? '8080');

server.listen(port, () => console.log(`Server is listening to port ${port}`));
