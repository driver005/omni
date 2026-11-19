//! JavaScript-visible façade over the Rust SDK. Only this layer knows about wasm-bindgen;
//! request construction and business rules stay in `sdk_client` and `sdk_domain`.

use js_sys::Promise;
use sdk_client::ApiClient;
use sdk_domain::resolve_theme_mode;
use wasm_bindgen::prelude::wasm_bindgen;
use wasm_bindgen::JsValue;
use wasm_bindgen_futures::future_to_promise;

/// Resolves a stored preference against system appearance. Pure Rust rule, callable from JS.
#[wasm_bindgen(js_name = resolveTheme)]
pub fn resolve_theme(preference: &str, system: &str) -> String {
    resolve_theme_mode(preference, system).to_owned()
}

/// The SDK handle imported by `visual` packages.
#[wasm_bindgen]
pub struct Sdk {
    client: ApiClient,
}

#[wasm_bindgen]
impl Sdk {
    /// Creates a client for `base_url`.
    #[wasm_bindgen(constructor)]
    pub fn new(base_url: &str) -> Result<Sdk, JsValue> {
        let client =
            ApiClient::new(base_url).map_err(|error| JsValue::from_str(&error.to_string()))?;
        Ok(Sdk { client })
    }

    /// Returns the configured server base URL.
    #[wasm_bindgen(js_name = baseUrl)]
    pub fn base_url(&self) -> String {
        self.client.base_url().to_owned()
    }

    /// Reads the server-owned theme preference through the generated route.
    #[wasm_bindgen(js_name = getThemePreference)]
    pub fn get_theme_preference(&self) -> Promise {
        let client = self.client.clone();
        future_to_promise(async move {
            let response = client
                .inner()
                .get_theme_preference()
                .await
                .map_err(|error| JsValue::from_str(&error.to_string()))?;
            Ok(JsValue::from_str(&response.into_inner()))
        })
    }

    /// Writes the server-owned theme preference through the generated route.
    #[wasm_bindgen(js_name = setThemePreference)]
    pub fn set_theme_preference(&self, mode: &str) -> Promise {
        let client = self.client.clone();
        let mode = mode.to_owned();
        future_to_promise(async move {
            client
                .inner()
                .put_theme_preference(&mode)
                .await
                .map_err(|error| JsValue::from_str(&error.to_string()))?;
            Ok(JsValue::from_bool(true))
        })
    }

    /// Fetches posts through the OpenAPI-generated request method.
    #[wasm_bindgen(js_name = listPosts)]
    pub fn list_posts(&self) -> Promise {
        let client = self.client.clone();
        future_to_promise(async move {
            let response = client
                .inner()
                .get_posts()
                .await
                .map_err(|error| JsValue::from_str(&error.to_string()))?;
            let body = format!(r#"{{"status":{}}}"#, response.status().as_u16());
            Ok(JsValue::from_str(&body))
        })
    }
}
