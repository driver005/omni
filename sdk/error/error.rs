//! Error types shared by every SDK crate.

use std::fmt;
use std::fmt::Formatter;

/// A recoverable failure in the SDK's request or mapping layers.
#[derive(Debug, Clone, PartialEq, Eq)]
#[non_exhaustive]
pub enum SdkError {
    /// The configured base URL could not be parsed or used.
    InvalidBaseUrl {
        /// The rejected URL value.
        url: String,
    },
    /// The server responded with a non-success status code.
    Http {
        /// HTTP status code.
        status: u16,
        /// Response body, truncated by the caller if needed.
        body: String,
    },
    /// The request failed before a response arrived.
    Transport {
        /// Underlying transport message.
        message: String,
    },
}

impl fmt::Display for SdkError {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidBaseUrl { url } => write!(formatter, "invalid base URL: {url}"),
            Self::Http { status, body } => write!(formatter, "HTTP {status}: {body}"),
            Self::Transport { message } => write!(formatter, "transport failure: {message}"),
        }
    }
}

impl std::error::Error for SdkError {}
