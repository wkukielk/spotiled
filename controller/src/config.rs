use serde::Deserialize;
use std::fs;
use std::path::Path;

#[derive(Deserialize)]
pub struct Config {
    pub bind_address: String,
}

impl Default for Config {
    fn default() -> Self {
        Config {
            bind_address: "[::]:5432".to_string(),
        }
    }
}

pub fn load(path: &Path) -> anyhow::Result<Config> {
    if !path.exists() {
        log::warn!("No config file at {}, using defaults", path.display());
        return Ok(Config::default());
    }
    let contents = fs::read_to_string(path)?;
    let config: Config = toml::from_str(&contents)?;
    Ok(config)
}
