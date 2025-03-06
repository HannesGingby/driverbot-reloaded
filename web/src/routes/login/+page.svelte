<script lang="ts">
  import { onMount, onDestroy } from "svelte";

  import Button from "../../components/global/button.svelte";
  import Input from "../../components/global/input.svelte";
  import Logo from "../../components/topbar/logo/logo.svelte";

  import ALargeSmall from "lucide-svelte/icons/a-large-small";
  import Eye from "lucide-svelte/icons/eye";
  import EyeOff from "lucide-svelte/icons/eye-off";

  import SpinnerJumper from "../../components/global/spinner-jumper.svelte";

  let inputUsername: string = $state("");
  let inputPassword: string = $state("");

  import { login } from "$lib/auth.js";
  import { connect } from "$lib/mqtt.js";
  import { authStore, mqttStore } from "$lib/store.svelte.js";
  import Account from "../../components/bottombar/account.svelte";

  function loginUser(username: string, password: string) {
    const success = login(username, password);

    if (success) {
      authStore.loginSuccess = true;
      connect(mqttStore.clusterURL, mqttStore.username, mqttStore.password);
    } else {
      authStore.loginSuccess = false;
      console.log("Failed to login");
    }
  }

  function connectUser() {
    connect(mqttStore.clusterURL, mqttStore.username, mqttStore.password);
  }

  // Dot loading signaling
  let dots: string = $state("");
  let dotsInterval: any;

  onMount(() => {
    let count = 0;
    dotsInterval = setInterval(() => {
      count = (count + 1) % 4; // cycle through
      dots = ".".repeat(count);
    }, 500);
  });

  onDestroy(() => {
    clearInterval(dotsInterval);
  });
</script>

<svelte:head>
  <title>Login</title>
</svelte:head>

{#if authStore.attemptingMQTTConnection}
  <main class="flex items-center justify-center h-[90vh] gap-3 pb-42">
    <p>Connecting to MQTT{dots}</p>
    <SpinnerJumper />
  </main>
{:else if !authStore.loggedIn && !authStore.connectedToMQTT}
  <main class="flex items-center justify-center flex-col h-[90vh] gap-3 pb-42">
    <div class="mb-8">
      <Logo />
    </div>
    <div class="w-xl">
      <h1 class="text-xl font-bold mb-6">Login</h1>
      <div class="w-full flex flex-col gap-2 mb-6">
        <Input
          placeholder="Username"
          Icon={ALargeSmall}
          ToggleIcon={ALargeSmall}
          bind:inputValue={inputUsername}
        />
        <Input
          placeholder="Password"
          type="password"
          Icon={EyeOff}
          ToggleIcon={Eye}
          bind:inputValue={inputPassword}
          iconToggle={true}
        />
      </div>
      <p class="text-sm text-text-200 mb-2">
        When logging in you will be automatically connected to the MQTT broker
        associated with your account.
      </p>
      <div class="w-[50%] mb-4">
        <Button
          action={() => loginUser(inputUsername, inputPassword)}
          style="normal"
          confirm={false}
          text="Login"
        />
      </div>
      {#if authStore.loginSuccess == false}
        <p class="text-sm text-bad absolute">Wrong credentials. Try again.</p>
      {/if}
    </div>
  </main>
{:else if authStore.loggedIn && !authStore.connectedToMQTT}
  <main class="flex items-center justify-center flex-col h-[90vh] gap-3 pb-42">
    <div class="mb-8">
      <Logo />
    </div>
    <div class="w-xl">
      <h1 class="text-xl font-bold mb-2">Logged in as {authStore.username}</h1>
      <div class="mb-6">
        <Account />
      </div>
      <p class="text-sm text-text-200 mb-2">
        When connecting you will be automatically connected to the MQTT broker
        associated with your account.
      </p>
      <div class="w-[50%] mb-4">
        <Button
          action={() => connectUser()}
          style="normal"
          confirm={false}
          text="Connect"
        />
      </div>
      {#if authStore.mqttSuccess == false}
        <p class="text-sm text-bad absolute">
          MQTT connection failed. You probably have the wrong broker credentials
          associated with your account. Try again.
        </p>
      {/if}
    </div>
  </main>
{/if}
