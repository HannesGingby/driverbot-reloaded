import { goto } from "$app/navigation";

// Routing
export function routeToPage(route: string, replaceState: boolean = false) {
	goto(`/${route}`, { replaceState })
}

// HEX to Rgba
type HEX = string;

export const durationUnitRegex = /[a-zA-Z]/;

export const calculateRgba = (color: HEX, opacity: number): string => {
	if (color[0] === '#') {
		color = color.slice(1);
	}

	if (color.length === 3) {
		let res = '';
		color.split('').forEach((c: string) => {
			res += c;
			res += c;
		});
		color = res;
	}

	const rgbValues = (color.match(/.{2}/g) || []).map((hex: HEX) => parseInt(hex, 16)).join(', ');

	return `rgba(${rgbValues}, ${opacity})`;
};

export const range = (size: number, startAt = 0) => [...Array(size).keys()].map((i) => i + startAt);


// Format time as hours:minutes:seconds
export function formatTime(timeInSeconds: number) {
	let hours = Math.floor(timeInSeconds / 3600);
	let minutes = Math.floor((timeInSeconds % 3600) / 60);
	let seconds = timeInSeconds % 60;

	return pad(hours) + ":" + pad(minutes) + ":" + pad(seconds);
}

export function formatDateTime(date: any) {
	let hours = date.getHours();
	let minutes = date.getMinutes();
	let seconds = date.getSeconds();

	return pad(hours) + ":" + pad(minutes) + ":" + pad(seconds);
}

// Add leading zero if single digit
export function pad(num: number) {
	return (num < 10 ? '0' : '') + num;
}
