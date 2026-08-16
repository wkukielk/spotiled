use super::{Controller, SongPipelineError};
use std::iter::Iterator;

pub(crate) struct CommandProcessor;

fn get_next_part<'a>(parts: &mut impl Iterator<Item=&'a str>) -> Result<&'a str, SongPipelineError> {
    parts.next().ok_or(SongPipelineError::IllFormedCommand)
}

impl CommandProcessor {
    pub(crate) fn new() -> Self {
        Self
    }

    pub(crate) async fn process<C: Controller>(
        &mut self,
        controller: &mut C,
        command: &str,
    ) -> Result<String, String> {
        match Self::dispatch(controller, command).await {
            Ok(()) => Ok(serde_json::json!({ "ack": "ok" }).to_string()),
            Err(err) => Err(serde_json::json!({ "error": err.to_string() }).to_string()),
        }
    }

    async fn dispatch<C: Controller>(
        controller: &mut C,
        command: &str,
    ) -> Result<(), SongPipelineError> {
        let mut parts = command.split_whitespace();
        match parts.next() {
            Some("connect") => {
                let hostname = get_next_part(&mut parts)?;
                controller.connect(hostname).await
            }
            Some("disconnect") => controller.disconnect(),
            Some("authorize") => {
                let auth_code = get_next_part(&mut parts)?;
                let code_verifier = get_next_part(&mut parts)?;
                controller.authorize(auth_code, code_verifier).await
            }
            _ => Err(SongPipelineError::UnknownCommand),
        }
    }
}

#[cfg(test)]
mod command_processor_tests;
