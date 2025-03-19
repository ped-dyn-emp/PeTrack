# Modify Dependencies

This guide covers setting up Docker and updating the Dockerfile. Please follow these steps whenever you want to modify dependencies.

---

### 1. Prerequisites

- Ensure Docker is installed. For Linux, you can install it via your package manager (e.g., `sudo pacman -S docker` for Arch Linux or `sudo apt install docker.io` for Ubuntu). For Windows or MacOS visit the [Docker Website](https://www.docker.com/get-started/).
- Start the Docker daemon with:
```bash
sudo systemctl start docker
```
- Check if Docker is properly set up by running:
```bash
docker run hello-world
```
- If you get any permission errors, adding your user to the Docker group might help (Note: you might have to restart your computer after that):
```bash
sudo usermod -aG docker $USER
```

### 2. Make your changes in the Dockerfile
- Modify `Dockerfile` under `container/ubuntu`

### 3. Modify the current Docker Image Version
In `.gitlab-ci.yml`, locate the image version defined in the variable `DOCKER_TAG`. We version them incrementally (e.g., if the latest is `v1`, change it to `v2`).

### 4. Build the image
- From the PeTrack root directory, go to the Dockerfile
```bash
cd container/ubuntu
```
- Build the image to ensure everything works as expected:
```bash
docker build -t jugit-registry.fz-juelich.de/ped-dyn-emp/petrack:vINSERT_NEW_VERSION_NUMBER_HERE .
```

### 5. Push the updated Docker image
- Log in to Docker Registry with your Gitlab account:
```bash
docker login jugit-registry.fz-juelich.de
```
- Push the new image:
```bash
docker push jugit-registry.fz-juelich.de/ped-dyn-emp/petrack:vINSERT_NEW_VERSION_NUMBER_HERE
```
