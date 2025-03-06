<script lang="ts">
  let { key, pressed = $bindable(), mousedown, mouseup } = $props();
</script>

<button
  class={pressed
    ? "w-20 h-12 bg-bg-600 transition-colors rounded-lg cursor-pointer"
    : "w-20 h-12 bg-bg-700 hover:bg-bg-650 active:bg-bg-600 transition-colors rounded-lg cursor-pointer"}
  onmousedown={mousedown}
  onmouseup={mouseup}
>
  {key.toUpperCase()}
</button>
