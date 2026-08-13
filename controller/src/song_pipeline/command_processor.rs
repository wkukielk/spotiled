use super::{Controller, SongPipelineError};

pub(crate) struct CommandProcessor;

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
                let hostname = parts.next().ok_or(SongPipelineError::MissingHostname)?;
                controller.connect(hostname).await
            }
            Some("disconnect") => controller.disconnect(),
            _ => Err(SongPipelineError::UnknownCommand),
        }
    }
}

#[cfg(test)]
mod command_processor_tests;
