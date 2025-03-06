<script lang="ts">
  let {
    action,
    Icon,
    iconHeight,
    text,
    color,
    textSize = 14,
    strokeWidth = "2",
    iconOffset = "0",
  } = $props();
</script>

<button
  onclick={action}
  class="flex items-center gap-1.5 hover:cursor-pointer group"
>
  <Icon
    size={iconHeight}
    class={"stroke-" + color + " mt-[" + iconOffset + "px]"}
    strokeWidth={parseFloat(strokeWidth)}
  />
  <p
    class={"text-" + color + " group-hover:underline"}
    style={"font-size: " + textSize.toString() + "px !important;"}
  >
    {text}
  </p>
</button>
