<script lang="ts">
  import { onMount } from "svelte";
  import { routeToPage } from "$lib/utils.js";
  import { gsap } from "gsap";

  import Topbar from "../../components/topbar/topbar.svelte";
  import CommandsSidebar from "../../components/commands-sidebar/commands-sidebar.svelte";
  import Map from "../../components/map/map.svelte";
  import Helpers from "../../components/helpers/helpers.svelte";
  import ShortcutHelpers from "../../components/helpers/shortcut-helpers.svelte";
  import InfoSidebar from "../../components/info-sidebar/info-sidebar.svelte";
  import Bottombar from "../../components/bottombar/bottombar.svelte";

  import Frown from "lucide-svelte/icons/frown";

  import {
    authStore,
    commandsSidebar,
    infoSidebar,
  } from "$lib/store.svelte.js";

  const dashboardAnimDuration = 0.6;

  onMount(() => {
    // Route to login page if not logged in or connected to MQTT
    if (!authStore.loggedIn || !authStore.connectedToMQTT) {
      routeToPage("login");
    }

    gsap.fromTo(
      "#gsap-top-down-opacity",
      { opacity: 0, y: -10 },
      { opacity: 1, y: 0, duration: dashboardAnimDuration, ease: "power1.inOut", stagger: 0.2, delay: 0.2 }
    );

    gsap.fromTo(
      "#gsap-bottom-up-opacity",
      { opacity: 0, y: 10 },
      { opacity: 1, y: 0, duration: dashboardAnimDuration, ease: "power1.inOut", stagger: 0.2, delay: 0.2 }
    );

    gsap.fromTo(
      "#gsap-left-right-opacity",
      { opacity: 0, x: -10 },
      { opacity: 1, x: 0, duration: dashboardAnimDuration, ease: "power1.inOut", stagger: 0.2 }
    );

    gsap.fromTo(
      "#gsap-right-left-opacity",
      { opacity: 0, x: 10 },
      { opacity: 1, x: 0, duration: dashboardAnimDuration, ease: "power1.inOut", stagger: 0.2 }
    );

    gsap.fromTo(
      "#gsap-fade-in",
      { opacity: 0 },
      { opacity: 1, duration: dashboardAnimDuration, ease: "power1.inOut", stagger: 0.2 }
    );
  });
</script>

<svelte:head>
  <title>Dashboard</title>
</svelte:head>

{#if authStore.loggedIn && authStore.connectedToMQTT}
  <main class="hidden md:block">
    <header id="gsap-top-down-opacity">
      <Topbar />
    </header>
    <div class="pt-[20px] pb-[24px] flex flex-nowrap h-[964px] justify-between">
      <div id="gsap-left-right-opacity" style="width: {commandsSidebar.width}px;">
        <CommandsSidebar />
      </div>
      <div id="gsap-fade-in" class="relative w-[calc(100%-644px)]">
        <Map styles="pt-12 pb-24 px-5 h-full w-full" />
        <Helpers styles="w-full px-8 absolute bottom-6" />
        <ShortcutHelpers styles="w-full px-8 absolute top-6" />
      </div>
      <div id="gsap-right-left-opacity" style="width: {infoSidebar.width}px;">
        <InfoSidebar />
      </div>
    </div>
    <nav id="gsap-bottom-up-opacity">
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
