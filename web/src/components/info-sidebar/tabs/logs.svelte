<script lang="ts">
  import { applicationLogs } from "$lib/store.svelte.js";

  import { onMount } from "svelte";
  import gsap from "gsap";
  import { fly } from 'svelte/transition';

  import LogItem from "./log-item.svelte";

  onMount(() => {
    gsap.fromTo(
      "#gsap-tab",
      { opacity: 0, y: -5 },
      { opacity: 1, y: 0, duration: 0.6, ease: "power3.out" }
    );
  });

  $effect(() => {
    gsap.fromTo(
      ".gsap-log-container",
      { opacity: 0, x: 5 },
      { opacity: 1, x: 0, duration: 0.6, ease: "power3.out", stagger: 0.05 }
    );
  });

  // Take last 30 + order
  let displayLogs = $derived(applicationLogs.logs.slice(-30).reverse());
</script>

<div id="gsap-tab" class="px-[13px]">
  <section>
    <div class="mb-8">
      <h1 class="text-lg mb-[8px] font-bold">Logs</h1>
      <p class="text-sm font-normal">Application logs</p>
    </div>
    <div>
      <div class="max-h-[500px] overflow-y-auto overflow-x-hidden flex flex-col gap-2 scroll-smooth">
        {#each displayLogs as log, index (log[1])}
          <div class="gsap-log-container" in:fly={{ y: -25, duration: 600, delay: index * 100 }}>
            <LogItem {log} />
          </div>
        {/each}
      </div>
      <div class="mt-4">
        <p class="text-text-200 text-sm">{applicationLogs.logs.length} logs</p>
      </div>
    </div>
  </section>
</div>
