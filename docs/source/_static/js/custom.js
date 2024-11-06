document.addEventListener("DOMContentLoaded", function () {
    let videos = document.getElementsByTagName("video");
    for (let i = 0; i < videos.length; i++) {
        const element = videos[i];
        element.setAttribute("controls", "")
    }
});