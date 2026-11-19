//! Owns the OpenAPI-derived client and applies SDK-wide request configuration.

use reqwest::header::HeaderMap;
use reqwest::header::HeaderValue;
use reqwest::header::AUTHORIZATION;
use sdk_api::Client as GeneratedClient;
use sdk_auth::Session;
use sdk_domain::normalize_base_url;
use sdk_error::SdkError;

/// Configured entry point for every generated server request.
#[derive(Clone)]
pub struct ApiClient {
    inner: GeneratedClient,
    base_url: String,
}

impl ApiClient {
    /// Creates an unauthenticated client for `base_url`.
    pub fn new(base_url: &str) -> Result<Self, SdkError> {
        Self::with_session(base_url, None)
    }

    /// Creates a client that sends `session` as a bearer token on every request.
    pub fn with_session(base_url: &str, session: Option<&Session>) -> Result<Self, SdkError> {
        let base_url = normalize_base_url(base_url);
        if base_url.is_empty() {
            return Err(SdkError::InvalidBaseUrl { url: base_url });
        }

        let mut headers = HeaderMap::new();
        if let Some(session) = session {
            let value = format!("Bearer {}", session.token());
            let header = HeaderValue::from_str(&value).map_err(|error| SdkError::Transport {
                message: error.to_string(),
            })?;
            headers.insert(AUTHORIZATION, header);
        }

        let http = reqwest::Client::builder()
            .default_headers(headers)
            .build()
            .map_err(|error| SdkError::Transport {
                message: error.to_string(),
            })?;

        Ok(Self {
            inner: GeneratedClient::new_with_client(&base_url, http),
            base_url,
        })
    }

    /// Returns the normalized server base URL.
    pub fn base_url(&self) -> &str {
        &self.base_url
    }

    /// Returns the generated client exposing one method per OpenAPI operation.
    pub fn inner(&self) -> &GeneratedClient {
        &self.inner
    }
}
