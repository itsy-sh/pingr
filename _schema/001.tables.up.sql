-- name: create-tests-table
CREATE TABLE IF NOT EXISTS tests (
    test_id TEXT PRIMARY KEY,
    test_name TEXT NOT NULL,
    test_type TEXT CHECK( test_type IN (
                                                'HTTP',
                                                'Prometheus',
                                                'TLS',
                                                'DNS',
                                                'Ping',
                                                'SSH',
                                                'TCP',
                                                'HTTPPush',
                                                'PrometheusPush'
                                            )
                                ),
    url TEXT NOT NULL,
    interval INTEGER NOT NULL,
    timeout  INTEGER NOT NULL,
    created_at TIMESTAMP NOT NULL,
    blob BLOB
);

-- name: create-logs-table
CREATE TABLE IF NOT EXISTS logs (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    test_id TEXT NOT NULL,
    status_id INTEGER NOT NULL,
    message TEXT,
    response_time INTEGER,
    created_at TIMESTAMP NOT NULL,
    FOREIGN KEY (test_id)
        REFERENCES tests (test_id)
    FOREIGN KEY (status_id)
        REFERENCES status_map (status_id)
);

-- name: create-status-map-table
CREATE TABLE IF NOT EXISTS status_map (
    status_id  INTEGER PRIMARY KEY NOT NULL,
    status_name TEXT NOT NULL,
    UNIQUE (status_id, status_name)
);

-- name: create-contacts-table
CREATE TABLE IF NOT EXISTS contacts (
    contact_id TEXT NOT NULL,
    contact_name TEXT NOT NULL,
    contact_type TEXT NOT NULL,
    contact_url TEXT NOT NULL
)

-- name: create-test-contact-mapper
CREATE TABLE IF NOT EXISTS test_contacts (
    test_id TEXT NOT NULL,
    contact_id TEXT NOT NULL,
    threshold INTEGER NOT NULL,
    UNIQUE (contact_id, test_id),
    FOREIGN KEY (test_id)
        REFERENCES tests (test_id)
    FOREIGN KEY (contact_id)
        REFERENCES contacts (contact_id)
)

-- name: init-status-mapper
INSERT INTO status_map(status_id, status_name)
VALUES
    (1, "Successful"),
    (2, "Error"),
    (3, "TimedOut"),
    (5, "Initialized"),
    (6, "Deleted");
