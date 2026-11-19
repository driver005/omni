//! Hand-written business logic. Request plumbing lives in `sdk_api`; presentation state
//! belongs in `visual`. This crate is the only place non-generated rules may be added.

/// Trims trailing slashes from a server base URL so generated paths never double up.
pub fn normalize_base_url(base_url: &str) -> String {
    base_url.trim_end_matches('/').to_owned()
}

/// Reports whether a session token is present and non-blank.
pub fn is_authenticated(token: &str) -> bool {
    !token.trim().is_empty()
}

/// Resolves a stored theme preference against the reported system appearance.
///
/// `preference` is one of `light`, `dark`, or `system`; `system` is `dark` or `light`.
/// Unknown values fall back to `light`, matching Ghost Admin's read-site fallback.
pub fn resolve_theme_mode(preference: &str, system: &str) -> &'static str {
    match preference {
        "dark" => "dark",
        "system" if system == "dark" => "dark",
        _ => "light",
    }
}

#[cfg(test)]
mod tests {
    use super::resolve_theme_mode;

    #[test]
    fn resolves_explicit_modes() {
        assert_eq!(resolve_theme_mode("dark", "light"), "dark");
        assert_eq!(resolve_theme_mode("light", "dark"), "light");
    }

    #[test]
    fn resolves_system_mode_from_reported_appearance() {
        assert_eq!(resolve_theme_mode("system", "dark"), "dark");
        assert_eq!(resolve_theme_mode("system", "light"), "light");
    }

    #[test]
    fn unknown_preference_falls_back_to_light() {
        assert_eq!(resolve_theme_mode("", "dark"), "light");
        assert_eq!(resolve_theme_mode("sepia", "dark"), "light");
    }
}
