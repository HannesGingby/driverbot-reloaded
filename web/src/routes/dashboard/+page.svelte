<script lang="ts">
  import { onMount } from "svelte";
  import { routeToPage } from "$lib/utils.js";

  import Topbar from "../../components/topbar/topbar.svelte";
  import CommandsSidebar from "../../components/commands-sidebar/commands-sidebar.svelte";
  import Map from "../../components/map/map.svelte";
  import Helpers from "../../components/helpers/helpers.svelte";
  import InfoSidebar from "../../components/info-sidebar/info-sidebar.svelte";
  import Bottombar from "../../components/bottombar/bottombar.svelte";

  import Frown from "lucide-svelte/icons/frown";

  import {
    authStore,
    commandsSidebar,
    infoSidebar,
  } from "$lib/store.svelte.js";

  // Route to login page if not logged in or connected to MQTT
  onMount(() => {
    if (!authStore.loggedIn || !authStore.connectedToMQTT) {
      routeToPage("login");
    }
  });
</script>

<svelte:head>
  <title>Dashboard</title>
</svelte:head>

{#if authStore.loggedIn && authStore.connectedToMQTT}
  <main class="hidden md:block">
    <header>
      <Topbar />
    </header>
    <div class="pt-[20px] pb-[24px] flex flex-nowrap h-[964px] justify-between">
      <div style="width: {commandsSidebar.width}px;">
        <CommandsSidebar />
      </div>
      <div class="flex flex-col items-center relative w-[calc(100%-644px)]">
        <Map styles="pt-12 pb-24 px-5 w-full h-full" />
        <Helpers styles="w-full px-8 absolute bottom-6" />
      </div>
      <div style="width: {infoSidebar.width}px;">
        <InfoSidebar />
      </div>
    </div>
    <nav>
      <Bottombar />
    </nav>
  </main>
  <div class="md:hidden mt-92">
    <Frown class="stroke-text-200" />
    <p class="mt-4">
      Small display sizes not supported. Increase your browser window width or
      try a different device.
    </p>
  </div>
{/if}
