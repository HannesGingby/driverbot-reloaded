<script lang="ts">
  import { onMount } from "svelte";
  import { co } from "$lib/store.svelte.js";
  import { mapProperties } from "$lib/store.svelte.js";
  import { roadTiles } from "$lib/roads.svelte.js";
  import type { RoadTile } from "$lib/roads.svelte.js";
  import RoadSprite from "./road-sprite.svelte";

  import SpinnerCircle from "../global/spinner-circle.svelte";

  let rowCount = $state(0);
  let columnCount = $state(0);
  let currentMouseCo: [number, number] = [0, 0];
  let gridElement: HTMLElement | undefined = $state();
  let loading = $state(true);

  let isPanning = $state(false);
  let panStart = { x: 0, y: 0 };
  let panOffset = { x: 0, y: 0 };

  // Track actual cell size after grid is rendered
  let actualCellWidth = $state(0);
  let actualCellHeight = $state(0);

  // Base sizes at default (100%) zoom
  const BASE_GAP_SIZE = 64;
  const BASE_DOT_SIZE = 4;

  // Initialize gapSize and dotSize
  let gapSize = $derived(Math.round(BASE_GAP_SIZE * (mapProperties.zoom / 100)));
  let dotSize = $derived(Math.round(BASE_DOT_SIZE * (mapProperties.zoom / 100)));
  let tileFontSize = $derived(Math.round(12 * (mapProperties.zoom / 100)));

  // Calculate the total cell size (gap + dot)
  let cellSize = $derived(gapSize + dotSize);

  const EXTRA_COLUMN_COUNT = 1;
  const EXTRA_ROW_COUNT = 1;

  const ORIGIN_X_OFFSET = -1;
  const ORIGIN_Y_OFFSET = 0;

  let originOffset = $state({ x: ORIGIN_X_OFFSET, y: ORIGIN_Y_OFFSET });

  // Function to update row and column counts based on container size
  function updateGridDimensions() {
    if (!gridElement) return;

    const rect = gridElement.getBoundingClientRect();
    const availableWidth = rect.width;
    const availableHeight = rect.height;

    // Calculate the number of columns and rows that can fit
    columnCount = Math.max(4, Math.floor(availableWidth / cellSize)) + EXTRA_COLUMN_COUNT;
    rowCount = Math.max(1, Math.floor(availableHeight / cellSize)) + EXTRA_ROW_COUNT;

    // measureActualCellSizes();
  }

  function getCoordinates(row: number, col: number): [number, number] {
    const startX = -Math.floor(columnCount / 2) - originOffset.x;
    const startY = Math.floor((rowCount - 1) / 2) - originOffset.y;
    return [startX + col, startY - row];
  }

  function coordsToCell(x: number, y: number, rowCount: number, columnCount: number) {
    const startX = -Math.floor(columnCount / 2) - originOffset.x;
    const startY = Math.floor((rowCount - 1) / 2) - originOffset.y;
    const col = x - startX;
    const row = startY - y;
    return [row, col] as const;
  }

  function tileAt(row: number, col: number): RoadTile | undefined {
    const [x,y] = getCoordinates(row, col);
    return roadTiles.find(t => t.x === x && t.y === y);
  }

  function handleMouseMove(e: MouseEvent) {
    if (!gridElement) return;
    const rect = gridElement.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Use the actual measured cell size for coordinate calculation
    const colSize = actualCellWidth > 0 ? actualCellWidth : cellSize;
    const rowSize = actualCellHeight > 0 ? actualCellHeight : cellSize;

    const startX = -Math.floor(columnCount / 2) - originOffset.x;
    const startY = Math.floor((rowCount - 1) / 2) - originOffset.y;
    const floatX = startX + x / colSize;
    const floatY = startY - y / rowSize;

    currentMouseCo = [
      parseFloat(floatX.toFixed(2)),
      parseFloat(floatY.toFixed(2)),
    ];
    co.mouseCo = currentMouseCo;

    if (!isPanning) return;
    panOffset.x = e.clientX - panStart.x;
    panOffset.y = e.clientY - panStart.y;

    // Update origin offset
    originOffset.x = Math.round(panOffset.x / cellSize);
    originOffset.y = Math.round(-panOffset.y / cellSize);
  }

  function handleMouseDown(e: MouseEvent) {
    if (e.button !== 1) return;        // only middle button
    isPanning = true;
    // record where we started (subtract existing offset so pan is cumulative)
    panStart.x = e.clientX - panOffset.x;
    panStart.y = e.clientY - panOffset.y;
    e.preventDefault();                // kill context menu / scrolling
  }

  function handleMouseUp(e: MouseEvent) {
    if (e.button !== 1) return;
    isPanning = false;
  }

  function handleScrolling(e: WheelEvent) {
    e.preventDefault();

    const zoomChange = e.deltaY > 0 ? -5 : 5; // Adjust zoom step as needed
    mapProperties.zoom = Math.min(200, Math.max(50, mapProperties.zoom + zoomChange));
  }

  function handleDoubleClick() {
    panOffset.x = 0;
    panOffset.y = 0;
    originOffset.x = ORIGIN_X_OFFSET;
    originOffset.y = ORIGIN_Y_OFFSET;
  }

  // Recalculate grid dimensions when zoom changes
  $effect(() => {
    updateGridDimensions();
  });

  // Init grid on mount
  onMount(() => {
    updateGridDimensions();
    loading = false;
  });
</script>

<svelte:window onresize={() => {
  updateGridDimensions();
}} />

{#if loading}
  <div class="flex h-full justify-center items-center pb-8">
    <SpinnerCircle size="50" />
  </div>
{:else if !loading}
<div
  bind:this={gridElement}
  class={"grid justify-center content-center " + (isPanning ? 'cursor-grabbing' : 'cursor-crosshair')}
  style="
    grid-template-columns: repeat({columnCount}, {cellSize - 2*dotSize}px);
    grid-template-rows: repeat({rowCount}, {cellSize - 2*dotSize}px);
    height: 100%;
  "
  onmousemove={handleMouseMove}
  onmousedown={handleMouseDown}
  onmouseup={handleMouseUp}
  onwheel={handleScrolling}
  ondblclick={handleDoubleClick}
  role="button"
  tabindex="0"
>
  {#each Array(rowCount) as _, row}
    {#each Array(columnCount) as _, col}
      {@const [x, y] = getCoordinates(row, col)}
      {@const tile = tileAt(row, col)}
      <div class="relative group flex items-center justify-center hover:bg-bg-700" style="width: {gapSize}px; height: {gapSize}px;">
        <div
          class="rounded-full w-full h-full bg-grid"
          style="width: {dotSize}px; height: {dotSize}px;"
        ></div>

        {#if tile}
          <div
            class="absolute w-full h-full"
          >
            <RoadSprite
              tile={tile}
              styles="w-full h-full"
            />
          </div>
        {/if}

        <div class="absolute top-0 left-1/2 transform -translate-x-1/2 -translate-y-8
                    bg-bg-700 text-text-100 px-2 py-1 rounded text-sm opacity-0
                    group-hover:opacity-100 transition-opacity whitespace-nowrap
                    pointer-events-none select-none">
          [{x}, {y}]
        </div>

        {#if x === 0 && y === 0}
        <div class="absolute w-full h-full inset-0 bg-pass-black">

        </div>
        <p class="absolute w-full pb-0.5
                     text-text-100 whitespace-nowrap
                    pointer-events-none select-none z-10 text-center pr-0.25" style="font-size: {tileFontSize}px;">START</p>
        {/if}
      </div>
    {/each}
  {/each}
</div>
{/if}
