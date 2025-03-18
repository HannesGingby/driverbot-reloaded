<script lang="ts">
    import { page } from '$app/state';
    import Logo from "./logo/logo.svelte";
    import LogIn from "lucide-svelte/icons/log-in";
    import LogOut from "lucide-svelte/icons/log-out";
    import Account from '../global/account.svelte';

    import { authStore } from "$lib/store.svelte.js";
    import { logout } from "$lib/auth.js";

    let accountDropdown = $state(false);
    let dropdownContainer: HTMLDivElement | undefined = $state(undefined);
    let toggleButton: HTMLButtonElement | undefined = $state(undefined);

    function handleWindowClick(event: MouseEvent) {
        // If dropdown is open and click is outside both the toggle button and the dropdown container, close the dropdown.
        if (
            accountDropdown &&
            toggleButton && 
            dropdownContainer &&
            !toggleButton.contains(event.target as Node) &&
            !dropdownContainer.contains(event.target as Node)
        ) {
            accountDropdown = false;
        }
    }
</script>

<svelte:window onclick={handleWindowClick} />

<nav id="gsap-top-down-opacity" class="flex items-center justify-between w-full h-[60px] px-[28px] bg-bg-800 rounded-lg">
    <div class="flex items-center gap-16">
        <Logo />
        <ul class="flex gap-6 text-text-200 *:hover:text-text-100">
            <li>
              <a href="/" class="link font-bold" class:text-text-100={page.url.pathname === '/'}>
                Home
              </a>
            </li>
            <li>
              <a href="/dashboard" class="link font-bold" class:text-text-100={page.url.pathname === '/dashboard'}>
                Dashboard
              </a>
            </li>
          </ul>
    </div>
    <div class="relative">
        {#if authStore.loggedIn}
            <button class="cursor-pointer" bind:this={toggleButton} onclick={() => (accountDropdown = !accountDropdown)}>
                <Account />
            </button>
            {#if accountDropdown}
                <div bind:this={dropdownContainer}>
                    <button class="absolute w-48 h-10 rounded-sm top-8 right-[-12px] bg-bg-700 flex justify-center items-center gap-2 cursor-pointer hover:bg-bg-600 transition-colors" onclick={() => logout()}>
                        <p>Logout</p>
                        <LogOut size="18" />
                    </button>
                </div>
            {/if}
        {:else}
            <a href="/login" class="flex items-center gap-2 cursor-pointer">
                <p>Login</p>
                <LogIn size="18" />
            </a>
        {/if}
    </div>
</nav>