import {CongeladoSdk, createThemeController} from '../packages/congelado-sdk/src/index';

const sdk = await CongeladoSdk.create(window.location.origin);
const controller = createThemeController(sdk);
await controller.init();

Object.assign(window, {__sdk: sdk, __theme: controller});
document.querySelector<HTMLDivElement>('#status')!.textContent = 'ready';
