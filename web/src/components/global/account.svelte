<script lang="ts">
  import { authStore } from "$lib/store.svelte.js";

  let avatar = $state("");
</script>

<a href="/" class="flex items-center gap-1.5">
  <img
    src={avatar}
    alt={authStore.username[0]}
    class="size-[20px] rounded-full bg-bg-600 flex items-center justify-center text-[12px]"
  />
  <p class="text-text-200">{authStore.username}</p>
</a>
