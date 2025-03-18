<script lang="ts">
  import { mqttStore } from "$lib/store.svelte.js";
  import { disconnect } from "$lib/mqtt.js";

  import { formatTime } from "$lib/utils.js";

  let uartBaudRate: number = 115200;
  let uartStatus: boolean = true;

  import DisplayvalueCopy from "../../global/displayvalue-copy.svelte";
  import HLine from "../../global/h-line.svelte";
  import Button from "../../global/button.svelte";

  import { onMount } from "svelte";
  import gsap from "gsap";

  onMount(() => {
    gsap.fromTo(
      "#gsap-tab",
      { opacity: 0, y: -5 },
      { opacity: 1, y: 0, duration: 0.6, ease: "power3.out" }
    );
  });
</script>

<div id="gsap-tab" class="px-[13px]">
  <section>
    <div class="mb-8">
      <h1 class="text-lg mb-[8px] font-bold">MQTT</h1>
      <p class="text-sm font-normal">
        Web client <b>connected for {formatTime(mqttStore.elapsedTime)}</b>
      </p>
    </div>
    <div class="**:text-sm flex flex-col gap-6 mb-8">
      <div>
        <p class="text-text-200 mb-3">Cluster URL:</p>
        <DisplayvalueCopy value={mqttStore.clusterBaseURL} />
      </div>
      <div>
        <p class="text-text-200 mb-0.5">Port:</p>
        <p>{mqttStore.port}</p>
      </div>
      <div>
        <p class="text-text-200 mb-0.5">Protocol:</p>
        <p>{mqttStore.protocol}</p>
      </div>
      <div>
        <p class="text-text-200 mb-0.5">Username:</p>
        <p>{mqttStore.username}</p>
      </div>
      <div>
        <p class="text-text-200 mb-0.5">ESP:</p>
        {#if mqttStore.espConnected}
          <p class="text-good">Ok</p>
        {:else if !mqttStore.espConnected}
          <p class="text-bad">Disconnected</p>
        {/if}
      </div>
    </div>
    <Button
      action={disconnect}
      style="destructive"
      confirm={true}
      text="Disconnect"
      confirmText="disconnect from MQTT"
    />
  </section>
  <HLine styles="mb-[20px] mt-[39px]" />
  <section>
    <h1 class="text-lg mb-4 font-bold">UART</h1>
    <div class="**:text-sm flex flex-col gap-6">
      <div>
        <p class="text-text-200 mb-0.5">Rate:</p>
        <p>{uartBaudRate}</p>
      </div>
      <div>
        <p class="text-text-200 mb-0.5">Status:</p>
        {#if uartStatus}
          <p class="text-good">Ok</p>
        {:else if !uartStatus}
          <p class="text-bad">Disconnected</p>
        {/if}
      </div>
    </div>
  </section>
</div>
