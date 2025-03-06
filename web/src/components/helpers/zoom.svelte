<script lang="ts">
  import { mapProperties } from "$lib/store.svelte.js";

  import Undo2 from "lucide-svelte/icons/undo-2";

  let isDragging = $state(false);
  let startY: number;
  let startZoom: number;

  function startDrag(event: MouseEvent) {
    isDragging = true;
    startY = event.clientY;
    startZoom = mapProperties.zoom;
    event.preventDefault(); // Prevent text selection
  }

  function onDrag(event: MouseEvent) {
    if (!isDragging) return;
    const delta = startY - event.clientY;
    mapProperties.zoom = Math.min(200, Math.max(50, startZoom + delta));
  }

  function stopDrag() {
    isDragging = false;
  }

  function reset() {
    mapProperties.zoom = 100;
  }
</script>

<svelte:window onmousemove={onDrag} onmouseup={stopDrag} ondblclick={reset} />

<div class="flex items-center gap-2 select-none">
  <p class="text-text-200 text-sm">Zoom:</p>
  <div class="relative w-[72px]">
    <input
      type="number"
      name="zoom"
      bind:value={mapProperties.zoom}
      min="50"
      max="200"
      class="w-full h-[26px] px-2 bg-bg-800 rounded-sm focus:outline-0 text-sm no-spinner cursor-ns-resize"
      onmousedown={startDrag}
    />
    <span
      class="absolute right-2 top-1/2 transform -translate-y-1/2 text-sm"
      style={isDragging
        ? "color: var(--color-text-100) !important;"
        : "color: var(--color-text-200) !important;"}
      >%
    </span>
  </div>
  <button onclick={reset}>
    <Undo2
      size="17.5"
      class="stroke-text-200 hover:stroke-text-100 transition-colors cursor-pointer"
    />
  </button>
</div>
