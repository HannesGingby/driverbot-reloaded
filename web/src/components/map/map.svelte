<script lang="ts">
  import Grid from "./grid.svelte";

  let { styles = "" } = $props();
</script>

<div class={styles}>
  <Grid />
</div>
