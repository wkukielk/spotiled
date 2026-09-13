use std::sync::Mutex;

#[derive(Clone, Debug, PartialEq)]
pub enum ControllerStatus {
    Starting,
    Running,
    Pairing { sas: String },
    Error(String),
}

static STATUS: Mutex<ControllerStatus> = Mutex::new(ControllerStatus::Starting);

pub fn set(status: ControllerStatus) {
    *STATUS.lock().unwrap() = status;
}

pub fn get() -> ControllerStatus {
    STATUS.lock().unwrap().clone()
}
