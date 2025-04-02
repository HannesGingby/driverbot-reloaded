<script lang="ts">
    let { value = $bindable(), onchange, name, min, max, step } = $props();
</script>

<input class="transition-colors" type="range" name={name} min={min} max={max} step={step} bind:value onchange={onchange} />

<style>
    input[type="range"] {
        -webkit-appearance: none;
        appearance: none;
        background-color: var(--color-bg-700);
        width: 100%;
        height: 10px;
        border-radius: 5px;
        cursor: pointer;
        outline: none;
        border: none;
        box-shadow: none;
    }

    input[type="range"]:hover {
        background-color: var(--color-bg-600);
    }

    /* Remove outline on focus */
    input[type="range"]:focus {
        outline: none;
        box-shadow: none;
    }

    /* WebKit browsers (Chrome, Safari) */
    input[type="range"]::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 18px;
        height: 18px;
        border-radius: 100%;
        background-color: var(--color-bg-500);
        border: none;
        cursor: pointer;
        box-shadow: none;
    }

    /* Firefox */
    input[type="range"]::-moz-range-thumb {
        width: 18px;
        height: 18px;
        border-radius: 100%;
        background-color: var(--color-bg-500);
        border: none;
        box-shadow: none;
    }
</style>