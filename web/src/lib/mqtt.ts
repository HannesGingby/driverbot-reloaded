import mqtt from "mqtt";

import { routeToPage } from "./utils.js";
import { authStore, mqttStore, movementData, logApplicationEvent, espData } from "./store.svelte.js";
import { roadTiles } from "./roads.svelte.js";

let client : any;
let connectionTimeoutId : any;

export function connect(clusterURL: string, username: string, password: string) {
  console.log("Connecting to MQTT broker...");
  authStore.attemptingMQTTConnection = true;

  if (clusterURL.startsWith("wss://")) {
    mqttStore.protocol = "WebSocket over TLS/SSL";
  } else {
    mqttStore.protocol = "Not WebSocket Secure";
  }

  // Generate random clientID
  if (mqttStore.clientID == "") {
    mqttStore.clientID = 'mqttjs_' + Math.random().toString(16).substr(2, 8);
  }

  client = mqtt.connect(clusterURL, {
    username: username,
    password: password,
    clientId: mqttStore.clientID,
    keepalive: 60,
    protocolId: 'MQTT',
    protocolVersion: 4,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000
  });

  client.on('connect', function () {
    logApplicationEvent("Connected to MQTT broker");
    authStore.connectedToMQTT = true;
    authStore.mqttSuccess = true;
    authStore.mqttError = undefined;

    setTimeout(() => {
      authStore.attemptingMQTTConnection = false;
      routeToPage("dashboard");

      mqttStore.time = new Date();
      updateConnectionTime();
    }, 600)
  });

  client.on('close', function() {
    logApplicationEvent("Disconnected from MQTT broker");
    authStore.connectedToMQTT = false;
    routeToPage("login");
  });

  client.on('error', function (error : any) {
    logApplicationEvent(error, true);
    authStore.mqttSuccess = false;
    authStore.mqttError = error;
  });

  client.on('message', function (topic: string, message : any) {
    if (topic == "esp") {
      // Parse JSON data
      let receivedEspData = JSON.parse(message);
      logApplicationEvent("ESP Data: " + JSON.stringify(receivedEspData));

      // Handle data
      let { x, y, heading, tileX, tileY, roadTile } = receivedEspData;

      if (typeof x === "number") espData.x = x;
      if (typeof y === "number") espData.y = y;
      if (typeof heading === "number") espData.heading = heading;
      if (typeof tileX === "number") espData.tileX = tileX;
      if (typeof tileY === "number") espData.tileY = tileY;
      if (roadTile !== undefined) espData.roadTile = roadTile;

      // Snap heading to 0, 90, 180, 270
      const snappedHeadings = [0, 90, 180, 270];
      heading = snappedHeadings.reduce((prev, curr) =>
        Math.abs(curr - heading) < Math.abs(prev - heading) ? curr : prev
      );

      // Update map
      roadTiles.push({type: roadTile, rotation: heading, x: tileX, y: tileY});
    }

    if (topic === "ping") {
      if (message.toString() === "1") {
        mqttStore.espConnected = true;
      }
    }
  });

  client.subscribe('movement');
  client.subscribe('task');
  client.subscribe('esp');
  client.subscribe('ping');

  // Set an interval for continuesly checking every 5 seconds if the ESP is connected or not, like a ping
  setInterval(() => {
    client.publish('ping', 0);

    // If the ping has not changed in 5 seconds, then the esp is disconnected (or has troubles)
    if (mqttStore.espConnected) {
      mqttStore.espConnected = false;
    }
  }, 5000);

  // Update connection time every second
  function updateConnectionTime() {
    const now = new Date();
    const elapsedTime = now.getTime() - mqttStore.time.getTime();

    mqttStore.elapsedTime = Math.floor(elapsedTime / 1000);
    connectionTimeoutId = setTimeout(updateConnectionTime, 1000);
  }
}

export function sendMovement() {
  const jsonMovement = JSON.stringify(movementData);

  if (jsonMovement != mqttStore.latestMovementSend) {
    logApplicationEvent("Sending MQTT user movement data: " + jsonMovement);
    client.publish('movement', jsonMovement);
    mqttStore.latestMovementSend = jsonMovement;
  }
}

export function sendMapUnknown(command: string, startPosX: number, startPosY: number, mapSizeX: number, mapSizeY: number) {
  logApplicationEvent("Sending MQTT task command: " + command);

  // Send json data
  const jsonData = {
    command: command,
    startPosX: startPosX,
    startPosY: startPosY,
    mapSizeX: mapSizeX,
    mapSizeY: mapSizeY,
  };

  client.publish('task', JSON.stringify(jsonData));
}

// Disconnect from the broker
export function disconnect() {
  logApplicationEvent("Disconnecting...");

  client.end();

  // Stop connection time counting
  clearTimeout(connectionTimeoutId);
}
