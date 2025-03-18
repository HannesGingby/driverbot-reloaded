<script lang="ts">
	import { onMount } from "svelte";
	import gsap from "gsap";

	onMount(() => {
		gsap.fromTo(
			"#gsap-tab",
			{ opacity: 0, y: -5 },
			{ opacity: 1, y: 0, duration: 0.6, ease: "power3.out" }
		);
	});
</script>


<div id="gsap-tab" class="px-[13px]">
	<section>
		<div class="mb-8">
			<h1 class="text-lg mb-[8px] font-bold">Mission</h1>
		</div>
	</section>
</div>