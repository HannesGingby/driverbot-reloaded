<script lang="ts">
  import type { RoadTile } from "$lib/roads.svelte.js";

  let { styles = "", tile}: { styles?: string, tile: RoadTile } = $props();
</script>

<!-- <div class="absolute top-0 left-0 w-full h-full bg-amber-200">

</div> -->
<img
  src={`/assets/road-tiles/${tile.type}.svg`}
  alt={tile.type}
  class={styles}
  style="
    max-width: unset !important;
    transform: rotate({tile.rotation}deg);
  "
/>
