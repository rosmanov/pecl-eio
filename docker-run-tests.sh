#!/bin/sh

docker compose up --build --remove-orphans --abort-on-container-failure
compose_exit_code=$?

printf '%s\n' "$(cat <<'REPORT'
 ____                       _
|  _ \ ___ _ __   ___  _ __| |_
| |_) / _ \ '_ \ / _ \| '__| __|
|  _ <  __/ |_) | (_) | |  | |_
|_| \_\___| .__/ \___/|_|   \__|
          |_|

REPORT
)"

final_exit_code=0

# A build/startup failure means the test suite did not complete successfully.
if [ "$compose_exit_code" -ne 0 ]; then
    printf >&2 'docker compose failed with exit code %s\n' "$compose_exit_code"
    final_exit_code=1
fi

# Check all containers that were actually created.
for container_id in $(docker-compose ps -q -a) ; do
    exit_code=$(docker inspect -f '{{.State.ExitCode}}' "$container_id")
    service_name=$(docker inspect \
        -f '{{ index .Config.Labels "com.docker.compose.service"}}' \
        "$container_id")

    printf >&2 'Service %s container %s exited with code %d\n' \
        "$service_name" "$container_id" "$exit_code"

    if [ "$exit_code" -ne 0 ]; then
        final_exit_code=1
    fi
done

exit "$final_exit_code"
