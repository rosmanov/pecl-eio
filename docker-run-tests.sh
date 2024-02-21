#!/bin/sh -

docker-compose up --build --remove-orphans

printf '%s\n' "$(cat <<REPORT
 ____                       _
|  _ \ ___ _ __   ___  _ __| |_
| |_) / _ \ '_ \ / _ \| '__| __|
|  _ <  __/ |_) | (_) | |  | |_
|_| \_\___| .__/ \___/|_|   \__|
          |_|


REPORT
)"

final_exit_code=0
for container_id in $(docker-compose ps -q -a) ; do
    exit_code=$(docker inspect -f '{{.State.ExitCode}}' $container_id)
    service_name=$(docker inspect -f '{{ index .Config.Labels "com.docker.compose.service"}}' "$container_id")

    printf "Service %s container %s exited with code %d\n" \
        "$service_name" "$container_id" "$exit_code"

    if [ "$exit_code" -ne 0 ]; then
        final_exit_code="$exit_code"
    fi
done

exit "$final_exit_code"
