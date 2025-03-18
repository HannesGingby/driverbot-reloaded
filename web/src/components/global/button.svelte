<script lang="ts">
  import { onDestroy } from "svelte";
  let { action, style, confirm, text, confirmText = "" } = $props();
  
  let displayConfirm = $state(false)

  let modal: Node | undefined = $state();

  $effect(() => {
    if (displayConfirm && modal && modal.parentNode !== document.body) {
    document.body.appendChild(modal);
  }});

  onDestroy(() => {
    if (modal && modal.parentNode === document.body) {
      document.body.removeChild(modal);
    }
  });

  function handleClick() {
    toggleConfirm(true);
  }

  function toggleConfirm(show: boolean) {
    if (confirm && show) {
      displayConfirm = true;
    } else if (!confirm && show) {
      action();
    } else {
      displayConfirm = false;
    }
  }

  function handleKeydown(event: KeyboardEvent) {
    if (event.key === "Escape") {
      toggleConfirm(false);
    }
  }
</script>

<svelte:window on:keydown={handleKeydown} />

{#if style === "normal"}
  <button
    type="button"
    class="cursor-pointer w-full h-[36px] bg-bg-700 text-text-100 text-sm rounded-lg hover:bg-bg-600 transition-colors"
    onclick={handleClick}
  >
    {text}
  </button>
{/if}

{#if style === "destructive"}
  <button
    type="button"
    class="cursor-pointer w-full h-[36px] bg-destructive-bg text-destructive-text text-sm rounded-lg hover:bg-destructive-hover-bg hover:text-destructive-hover-text transition-colors"
    onclick={handleClick}
  >
    {text}
  </button>
{/if}

{#if displayConfirm}
  <div bind:this={modal} id="overlay" class="fixed top-0 left-0 w-screen h-screen z-50 **:text-text-100">
    <button
      type="button"
      class="absolute inset-0 w-full h-full bg-pass-black cursor-default"
      onclick={() => toggleConfirm(false)}
      aria-label="Close dialog"
    ></button>
    <dialog
      aria-labelledby="confirm-dialog-title"
      class="fixed left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 w-xl bg-transparent"
      open
    >
      <div class="mb-48">
        <h1 id="confirm-dialog-title" class="mb-4">
          Are you sure you want to {confirmText}?
        </h1>
        <div class="flex gap-4">
          <button
            class="w-full h-[36px] bg-bg-700 hover:bg-bg-600 transition-colors rounded-lg cursor-pointer text-sm"
            type="button"
            onclick={() => {
              action();
              toggleConfirm(false);
            }}
          >
            Yes
          </button>
          <button
            class="w-full h-[36px] bg-bg-700 hover:bg-bg-600 transition-colors rounded-lg cursor-pointer text-sm"
            type="button"
            onclick={() => toggleConfirm(false)}
          >
            No, cancel &#40;Esc&#41;
          </button>
        </div>
      </div>
    </dialog>
  </div>
{/if}