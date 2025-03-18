<script lang="ts">
  import { onMount } from "svelte";
  import { co } from "$lib/store.svelte.js";
  import { mapProperties } from "$lib/store.svelte.js";

  import SpinnerCircle from "../global/spinner-circle.svelte";

  let rowCount = $state(0);
  let columnCount = $state(0);
  let currentMouseCo: [number, number] = [0, 0];
  let gridElement: HTMLElement | undefined = $state();
  let loading = $state(true);

  // Base sizes at default (100%) zoom
  const BASE_GAP_SIZE = 64;
  const BASE_DOT_SIZE = 4;

  // Initialize gapSize and dotSize
  let gapSize = $state(Math.round(BASE_GAP_SIZE * (mapProperties.zoom / 100)));
  let dotSize = $state(Math.round(BASE_DOT_SIZE * (mapProperties.zoom / 100)));

  // Function to update row and column counts based on container size
  function updateGridDimensions() {
    if (!gridElement) return;

    const rect = gridElement.getBoundingClientRect();
    const availableWidth = rect.width;
    const availableHeight = rect.height;

    // Calculate the number of columns and rows based on available space
    columnCount = Math.max(4, Math.floor(availableWidth / (dotSize + gapSize)));
    rowCount = Math.max(1, Math.floor(availableHeight / (dotSize + gapSize)));
  }

  function getCoordinates(row: number, col: number): [number, number] {
    const startX = -Math.floor(columnCount / 2);
    const startY = Math.floor((rowCount - 1) / 2);
    return [startX + col, startY - row];
  }

  function handleMouseMove(event: MouseEvent) {
    if (!gridElement) return;
    const rect = gridElement.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    const colSize = gapSize + dotSize;
    const rowSize = gapSize + dotSize;

    const startX = -Math.floor(columnCount / 2);
    const startY = Math.floor((rowCount - 1) / 2);
    const floatX = startX + x / colSize;
    const floatY = startY - y / rowSize;

    currentMouseCo = [
      parseFloat(floatX.toFixed(2)),
      parseFloat(floatY.toFixed(2)),
    ];
    co.mouseCo = currentMouseCo;
  }

  // Watch for changes in zoom level
  $effect(() => {
    gapSize = Math.round(BASE_GAP_SIZE * (mapProperties.zoom / 100));
    dotSize = Math.round(BASE_DOT_SIZE * (mapProperties.zoom / 100));
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
    onmousemove={handleMouseMove}
  >
    {#each Array(rowCount) as _, row}
      {#each Array(columnCount) as _, col}
        {@const [x, y] = getCoordinates(row, col)}
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
            [{x}, {y}]
          </div>
        </div>
      {/each}
    {/each}
  </figure>
{/if}
