use super::*;

#[test]
fn reloading_the_same_directory_yields_the_same_fingerprint() {
    let dir = tempfile::tempdir().unwrap();
    let (_, first) = load_or_generate(dir.path(), "test-controller").unwrap();
    let (_, second) = load_or_generate(dir.path(), "test-controller").unwrap();
    assert_eq!(first, second);
}

#[test]
fn different_directories_get_different_identities() {
    let dir_a = tempfile::tempdir().unwrap();
    let dir_b = tempfile::tempdir().unwrap();
    let (_, a) = load_or_generate(dir_a.path(), "test-controller").unwrap();
    let (_, b) = load_or_generate(dir_b.path(), "test-controller").unwrap();
    assert_ne!(a, b);
}
