use super::{Controller, SongPipelineError};

pub(crate) struct CommandProcessor<C: Controller> {
    controller: C,
}

impl<C: Controller> CommandProcessor<C> {
    pub(crate) fn new(controller: C) -> Self {
        Self { controller }
    }

    pub(crate) async fn process(&mut self, command: &str) -> Result<String, String> {
        match self.dispatch(command).await {
            Ok(()) => Ok(serde_json::json!({ "ack": "ok" }).to_string()),
            Err(err) => Err(serde_json::json!({ "error": err.to_string() }).to_string()),
        }
    }

    async fn dispatch(&mut self, command: &str) -> Result<(), SongPipelineError> {
        let mut parts = command.split_whitespace();
        match parts.next() {
            Some("connect") => {
                let hostname = parts.next().ok_or(SongPipelineError::MissingHostname)?;
                self.controller.connect(hostname).await
            }
            Some("disconnect") => self.controller.disconnect(),
            _ => Err(SongPipelineError::UnknownCommand),
        }
    }
}

#[cfg(test)]
mod command_processor_tests;
