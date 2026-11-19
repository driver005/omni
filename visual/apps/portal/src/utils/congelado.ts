/**
 * Portal's opt-in bridge to the Rust SDK. Enabled with `data-congelado="true"` on the
 * `script[data-ghost]` tag so existing sites are unaffected.
 */
import {CongeladoSdk} from '@tryghost/congelado-sdk';

export async function probeCongeladoSdk(baseUrl: string): Promise<number | undefined> {
    const sdk = await CongeladoSdk.create(baseUrl);
    const result = await sdk.listPosts();
    return result.status;
}
