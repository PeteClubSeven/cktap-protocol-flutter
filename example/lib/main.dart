import 'dart:async';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:cktap_protocol/cktap_protocol.dart' as cktap_protocol;
import 'package:ffi/ffi.dart';
import 'package:flutter/material.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

void main() {
  runApp(const MyApp());
}

IsoDep? isoDep;

int transceiveIsoDep(Pointer<Uint8> rawData, int dataLength) {
  if (isoDep == null) {
    return -1;
  }
  if (isoDep!.maxTransceiveLength != 0 && isoDep!.maxTransceiveLength < dataLength)
  {
    return -2;
  }

  Uint8List dataToTransmit = rawData.asTypedList(dataLength);
  isoDep!.transceive(data: dataToTransmit).then((value) {
    Pointer<Uint8> allocation = cktap_protocol.allocateResponse(value.lengthInBytes);
    if (allocation.address == 0)
    {
      // something went real wrong
      return;
    }

    Uint8List list =  allocation.asTypedList(value.length);
    list.setAll(0, value);

    cktap_protocol.finalizeResponse();
  }).catchError((err) {
    print('Error: $err'); // Prints 401.
  }, test: (error) {
    return error is int && error >= 400;
  });

  return 0;
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late int sumResult;
  late Future<int> sumAsyncResult;

  @override
  void initState() {
    super.initState();
    sumResult = cktap_protocol.sum(1, 2);
    sumAsyncResult = cktap_protocol.sumAsync(3, 4);
    NfcManager.instance.startSession(
      onDiscovered: (NfcTag tag) async {
        isoDep = IsoDep.from(tag);
        if (isoDep != null) {
          // Check if this breaks because it never returns.
          int result = cktap_protocol.createCKTapCard(Pointer.fromFunction(transceiveIsoDep, 1));
          if (result != 0)
          {
            print(result);
          }
        }
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    const textStyle = TextStyle(fontSize: 25);
    const spacerSmall = SizedBox(height: 10);
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Native Packages'),
        ),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              children: [
                const Text(
                  'This calls a native function through FFI that is shipped as source in the package. '
                  'The native code is built as part of the Flutter Runner build.',
                  style: textStyle,
                  textAlign: TextAlign.center,
                ),
                spacerSmall,
                Text(
                  'sum(1, 2) = $sumResult',
                  style: textStyle,
                  textAlign: TextAlign.center,
                ),
                spacerSmall,
                FutureBuilder<int>(
                  future: sumAsyncResult,
                  builder: (BuildContext context, AsyncSnapshot<int> value) {
                    final displayValue =
                        (value.hasData) ? value.data : 'loading';
                    return Text(
                      'await sumAsync(3, 4) = $displayValue',
                      style: textStyle,
                      textAlign: TextAlign.center,
                    );
                  },
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
