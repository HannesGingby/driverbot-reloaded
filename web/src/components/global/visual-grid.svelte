<script lang="ts">
  import { onMount } from "svelte";

  import SpinnerCircle from "../global/spinner-circle.svelte";

  let rowCount = $state(0);
  let columnCount = $state(0);
  let gapSize = 48;
  let dotSize = 4;
  let gridElement: HTMLElement | undefined = $state();
  let loading = $state(true);

  function updateGridDimensions() {
    if (!gridElement) return;

    const rect = gridElement.getBoundingClientRect();
    const availableWidth = rect.width;
    const availableHeight = rect.height;

    // Calculate the number of columns and rows based on available space
    columnCount = Math.max(4, Math.floor(availableWidth / (dotSize + gapSize)));
    rowCount = Math.max(1, Math.floor(availableHeight / (dotSize + gapSize)));
  }
  
  $effect(() => {
    updateGridDimensions();
  });

  // Init grid on mount
  onMount(() => {
    updateGridDimensions();
    loading = false;
  });
</script>

<svelte:window onresize={updateGridDimensions} />

{#if loading}
  <div class="flex h-full justify-center items-center pb-8">
    <SpinnerCircle size="50" />
  </div>
{:else if !loading}
  <figure
    class="grid cursor-crosshair"
    style="gap: {gapSize}px; height: 100%;"
    bind:this={gridElement}
  >
    {#each Array(rowCount) as _, row}
      {#each Array(columnCount) as _, col}
        <div
          class="bg-grid rounded-full relative group cursor-default"
          style="
            grid-row: {row + 1};
            grid-column: {col + 1};
            width: {dotSize}px;
            height: {dotSize}px;
          "
        >
          <div
            class="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2"
            style="width: {dotSize * 5}px; height: {dotSize * 5}px;"
          ></div>
          <div
            class="absolute top-0 left-1/2 transform -translate-x-1/2 -translate-y-8
                    bg-bg-700 text-text-100 px-2 py-1 rounded text-sm opacity-0
                    group-hover:opacity-100 transition-opacity whitespace-nowrap pointer-events-none select-none"
          >
            [{col - Math.floor(columnCount / 2)}, {Math.floor((rowCount - 1) / 2) - row}]
          </div>
        </div>
      {/each}
    {/each}
  </figure>
{/if}