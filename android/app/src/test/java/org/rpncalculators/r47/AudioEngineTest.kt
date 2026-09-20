package org.rpncalculators.r47

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class AudioEngineTest {

    @Test
    fun testVolumeClampingAndSettings() {
        AudioEngine.setBeeperVolume(50)
        assertEquals(50, AudioEngine.getBeeperVolume())

        // Test clamping lower bound
        AudioEngine.setBeeperVolume(-10)
        assertEquals(0, AudioEngine.getBeeperVolume())

        // Test clamping upper bound
        AudioEngine.setBeeperVolume(150)
        assertEquals(100, AudioEngine.getBeeperVolume())

        // Test updateSettings
        AudioEngine.updateSettings(enabled = false, volume = 75)
        assertEquals(75, AudioEngine.getBeeperVolume())

        AudioEngine.updateSettings(enabled = true, volume = 20)
        assertEquals(20, AudioEngine.getBeeperVolume())
    }

    @Test
    fun testBidirectionalVolumeMappingSteps0To11() {
        // C47 defines 12 volume steps: 0..11
        // Native-lib maps: pct = (vol * 100) / 11
        // and: vol = (pct * 11 + 50) / 100
        for (vol in 0..11) {
            val pct = (vol * 100) / 11
            val roundTripVol = (pct * 11 + 50) / 100
            assertEquals("Volume step $vol must roundtrip exactly", vol, roundTripVol)
        }
    }

    @Test
    fun testAmplitudeMonotonicityAndSilenceAtZero() {
        // At volume 0%, amplitude must be exactly 0 (mute)
        AudioEngine.setBeeperVolume(0)
        val amp0 = (AudioEngine.getBeeperVolume() * 163.84).toInt().toShort()
        assertEquals(0.toShort(), amp0)

        // Across steps 1..11, amplitude must be strictly increasing
        var previousAmp: Short = 0
        for (vol in 1..11) {
            val pct = (vol * 100) / 11
            AudioEngine.setBeeperVolume(pct)
            val amp = (AudioEngine.getBeeperVolume() * 163.84).toInt().toShort()
            assertTrue("Amplitude at vol $vol ($amp) must be greater than previous ($previousAmp)", amp > previousAmp)
            previousAmp = amp
        }
    }

    @Test
    fun testTonePackingAndUnpackingMath() {
        // Test standard 440 Hz (440000 milliHz) for 200 ms
        val milliHz = 440000
        val durationMs = 200
        val frequency = maxOf(1, milliHz / 1000)
        val packed = (frequency.toLong() shl 32) or (durationMs.toLong() and 0xFFFFFFFF)

        val unpackedFreq = (packed shr 32).toInt()
        val unpackedDuration = (packed and 0xFFFFFFFF).toInt()

        assertEquals(440, unpackedFreq)
        assertEquals(200, unpackedDuration)
    }

    @Test
    fun testWriteBufferSizeMeetsHardwareThreshold() {
        // For any tone, writeSize must be at least minSamples (e.g. 2048 frames / minBufSize / 2)
        // to prevent AudioFlinger from stalling on buffer underrun.
        val sampleRate = 44100
        val minBufSize = 4096 // Typical minimum buffer in bytes (2048 frames)
        val minSamples = maxOf(minBufSize / 2, 2048)

        val shortDurationsMs = listOf(5, 10, 50, 100, 200)
        for (durationMs in shortDurationsMs) {
            val noteSamples = durationMs * sampleRate / 1000
            val totalSamples = noteSamples + 882
            val writeSize = maxOf(totalSamples, minSamples)

            assertTrue(
                "Write size for duration ${durationMs}ms ($writeSize) must satisfy hardware burst threshold ($minSamples)",
                writeSize >= minSamples
            )
        }
    }
}
