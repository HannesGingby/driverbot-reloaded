import { disconnect } from "./mqtt.js";
import { authStore, mqttStore, resetMqttStore, resetAuthStore } from "./store.svelte.js";

export function login(username: string, password: string): boolean {
  // todo

  // todo: get MQTT collection associated with the user
  const clusterURL = "wss://n39420ee.ala.eu-central-1.emqxsl.com:8084/mqtt";
  const url = new URL(clusterURL);
  const clusterBaseURL = url.hostname;
  const clusterPort = parseInt(url.port);

  mqttStore.clusterURL = clusterURL;
  mqttStore.clusterBaseURL = clusterBaseURL;
  mqttStore.port = clusterPort;
  mqttStore.username = "replace_this";
  mqttStore.password = "replace_this";

  authStore.loggedIn = true;
  authStore.username = "Hannes";
  return true;
}

export function logout() {
  // todo

  disconnect();

  resetAuthStore();
  resetMqttStore();
}