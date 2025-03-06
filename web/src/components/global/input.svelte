<script lang="ts">
  let {
    placeholder,
    type = "text",
    Icon,
    inputValue = $bindable(),
    iconToggle = false,
    ToggleIcon,
  } = $props();

  let toggle = $state(false);
  let inputType = $state(type);
</script>

<div class="w-full bg-bg-700 rounded-lg px-4 h-[36px] flex justify-between">
  <input
    type={inputType}
    {placeholder}
    bind:value={inputValue}
    class="focus:outline-0 text-sm w-full"
  />
  {#if iconToggle}
    <button
      onclick={() => {
        toggle ? (toggle = false) : (toggle = true);
        if (type === "password") {
          toggle ? (inputType = "text") : (inputType = "password");
        }
      }}
      class="cursor-pointer"
    >
      {#if toggle}
        <ToggleIcon size="17.5" />
      {:else}
        <Icon size="17.5" />
      {/if}
    </button>
  {:else}
    <button>
      <Icon size="17.5" />
    </button>
  {/if}
</div>
