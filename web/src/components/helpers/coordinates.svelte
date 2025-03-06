<script lang="ts">
  let { x = 0, y = 0 } = $props();
</script>

<p class="inline-block w-30 mr-2 text-text-300">
  x: {x}, y: {y}
</p>
<p class="inline-block text-text-300">
  tile: &#91;{Math.floor(x)}, {Math.floor(y)}&#93;
</p>
