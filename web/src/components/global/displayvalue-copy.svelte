<script lang="ts">
  let { value } = $props();

  import Copy from "lucide-svelte/icons/copy";

  function copyValue(value: string) {
    navigator.clipboard.writeText(value);
  }
</script>

<div
  class="flex justify-between w-full h-[36px] items-center px-[12px] bg-bg-700 rounded-lg"
>
  <p>{value}</p>
  <button class="cursor-pointer mt-0.5" onclick={() => copyValue(value)}>
    <Copy size="17.5" />
  </button>
</div>
