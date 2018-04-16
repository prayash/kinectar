const fs = require('fs')
const path = require('path')
import * as THREE from 'three'
import rafEngine from 'raf-loop'
import setup from './setup'

const { renderer, camera, scene, updateControls } = setup()

const video = document.createElement('video')
video.addEventListener(
  'loadedmetadata',
  event => {
    const texture = new THREE.VideoTexture(video)
    texture.minFilter = THREE.NearestFilter

    const width = 640
    const height = 480
    const nearClipping = 850
    const farClipping = 4000

    const geometry = new THREE.BufferGeometry()
    const vertices = new Float32Array(width * height * 3)

    for (var i = 0, j = 0, l = vertices.length; i < l; i += 3, j++) {
      vertices[i] = j % width
      vertices[i + 1] = Math.floor(j / width)
    }

    geometry.addAttribute('position', new THREE.BufferAttribute(vertices, 3))

    // Create our vertex/fragment shaders
    const material = new THREE.ShaderMaterial({
      uniforms: {
        map: { value: texture },
        width: { value: width },
        height: { value: height },
        nearClipping: { value: nearClipping },
        farClipping: { value: farClipping },

        pointSize: { value: 2 },
        zOffset: { value: 1000 }
      },
      vertexShader: fs.readFileSync(
        path.join(__dirname, 'shaders/shader.vert'),
        'utf8'
      ),
      fragmentShader: fs.readFileSync(
        path.join(__dirname, 'shaders/shader.frag'),
        'utf8'
      ),
      blending: THREE.AdditiveBlending,
      depthTest: false,
      depthWrite: false,
      transparent: true
    })

    // Setup our mesh
    const mesh = new THREE.Points(geometry, material)
    scene.add(mesh)
  },
  false
)

video.loop = true
video.muted = true
video.src = 'assets/kinect2.webm'
video.setAttribute('webkit-playsinline', 'webkit-playsinline')
video.play()

let time = 0
rafEngine(dt => {
  // update time
  time += dt / 1000

  // render
  updateControls()
  renderer.render(scene, camera)
}).start()
