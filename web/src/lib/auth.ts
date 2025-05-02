import { disconnect } from "./mqtt.js";
import { authStore, mqttStore, resetMqttStore, resetAuthStore, logApplicationEvent } from "./store.svelte.js";
import PocketBase from "pocketbase";

// Init PocketBase
const pb = new PocketBase('http://localhost:8090');

/**
 * Login user with PocketBase and retrieve MQTT connection information
 */
export async function login(username: string, password: string): Promise<boolean> {
  try {
    // Authenticate with PocketBase
    const authData = await pb.collection('users').authWithPassword(username, password);

    // Set auth store values
    authStore.loggedIn = true;
    authStore.username = username;
    authStore.userId = authData.record.id;

    // Try to get user's MQTT connection details from PocketBase
    try {
      const mqttDetails = await pb.collection('mqtt_connections').getFirstListItem(`user="${authData.record.id}"`);

      if (mqttDetails) {
        // Set MQTT connection details from PocketBase
        setMQTTConfig(mqttDetails);
        logApplicationEvent("MQTT connection details retrieved from database");
      }
    } catch (mqttError: any) {
      // Error fetching MQTT details
      logApplicationEvent(`Error fetching MQTT connection config from db: ${mqttError.message}`, true);
    }

    logApplicationEvent("Login successful");
    return true;
  } catch (error: any) {
    logApplicationEvent(`Login failed: ${error.message}`, true);
    return false;
  }
}

/**
 * Set MQTT connection details from a PocketBase record
 */
function setMQTTConfig(mqttRecord: any) {
  const url = new URL(mqttRecord.cluster_url);
  const clusterBaseURL = url.hostname;
  const clusterPort = parseInt(url.port);

  mqttStore.clusterURL = mqttRecord.cluster_url;
  mqttStore.clusterBaseURL = clusterBaseURL;
  mqttStore.port = clusterPort;
  mqttStore.username = mqttRecord.mqtt_username;
  mqttStore.password = mqttRecord.mqtt_password;
}

/**
 * Logout user and clean up
 */
export function logout() {
  // Disconnect from MQTT
  disconnect();
  logApplicationEvent("Disconnected from MQTT");

  // Logout from PocketBase
  pb.authStore.clear();

  // Reset local stores
  resetAuthStore();
  resetMqttStore();
  logApplicationEvent("Resetting session data");
}

/**
 * Check if user is already logged in from a previous session
 */
export function checkExistingSession(): boolean {
  if (pb.authStore.isValid) {
    // We have a valid PocketBase session
    const userData = pb.authStore.record;

    // Set auth store data
    authStore.loggedIn = true;
    authStore.username = userData?.username || userData?.email;
    authStore.userId = userData?.id;

    // Try to load MQTT settings
    pb.collection('mqtt_connections').getFirstListItem(`user="${userData?.id}"`)
      .then(mqttDetails => {
        setMQTTConfig(mqttDetails);
        logApplicationEvent("Restored previous session with MQTT configuration");
      })
      .catch(() => {
        // Use default MQTT settings if none found
        logApplicationEvent("No MQTT configuration found");
      });

    return true;
  }

  return false;
}
