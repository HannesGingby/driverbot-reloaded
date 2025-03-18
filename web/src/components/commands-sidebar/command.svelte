<script lang="ts">
  let { action, label, Icon, Svg, svgColor } = $props();
</script>

<button
  class="flex items-center justify-between bg-bg-700 hover:bg-bg-650 transition-colors cursor-pointer w-full h-[50px] rounded-lg px-[20px]"
  onclick={action}
>
  <div class="flex items-center gap-1.5">
    <p class="mb-0.5 text-nowrap">{label}</p>
    <Icon size="18px" class="stroke-text-100" />
  </div>
  <Svg color={svgColor} />
</button>
