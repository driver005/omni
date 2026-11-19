//! Authentication state for Ghost Admin requests.

/// A bearer/session token applied to every outbound request.
#[derive(Debug, Clone, PartialEq, Eq)]
#[non_exhaustive]
pub struct Session {
    token: String,
}

impl Session {
    /// Creates a session from `token`.
    pub fn new(token: &str) -> Self {
        Self {
            token: token.to_owned(),
        }
    }

    /// Returns the token value.
    pub fn token(&self) -> &str {
        &self.token
    }
}
