
## About the data

Ten participants (3 females, 7 males) were recruited for this case-study. Participants were equipped with a Shimmer 3 (Shimmer, Dublin, Ireland) inertial measurement unit (IMU) on their dominant foot. Each participant completed 20 countermovement jumps with acceptable form, 20 jumps with their legs bending during flight and 20 jumps with a stumble upon landing. The resulting 3-class dataset consists of 200 instances of IMU data in the acceptable form class and, due to Bluetooth dropping twice during data collection, 199 instances of IMU data in both the 'legs bending' and 'stumble on landing' classes. The data is further cropped (i.e. trim the uneventful parts of the signals) and resampled to equal length.

## The author

The data was collected and contributed to our study by the courtesy of [Martin](https://www.researchgate.net/profile/Martin_Oreilly4). He is currently working on the [Output Sports](http://www.outputsports.com/) project where most of the data is time series.

## Additional instructions

Mr-SEQL has a parameter which is the maximum size of the sliding window. By default it is the length of the time series. However, in the case of variable length data (e.g. Jump and JumpCropped) this parameter must be set by user. The following commands reproduce our experiments on the Jump/JumpCropped data:


```
mkdir saxdir
./sax_convert -i Jump_TRAIN -o saxdir/sax.train -n 64 -N 1000 -w 16 -a 4 -m 1 > saxdir/config
./sax_convert -i Jump_TEST -o saxdir/sax.test -n 64 -N 1000 -w 16 -a 4 -m 1
./mr_seql -t saxdir/sax.train -T saxdir/sax.test -o saxdir

```

Since JumpResampled has time series with equal length, the general instructions on the main page should be sufficient to reproduce the results.
