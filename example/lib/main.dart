import 'dart:async';

import 'package:cktap_protocol/cktap_protocol.dart';
import 'package:flutter/material.dart';
import 'package:nfc_manager/nfc_manager.dart';

void main() {
  runApp(const MyApp());
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
    sumResult = 1;
    sumAsyncResult = Future(() => 2);
    NfcManager.instance.startSession(
      onDiscovered: (NfcTag tag) async {
        var card = await createCKTapCard(tag);
        if (card.isTapsigner) {
          var tapsigner = card.toTapsigner();
          
        }
        else
        {
          var satscard = card.toSatscard();
          if (satscard != null) {
            Navigator. of(context).push(
              MaterialPageRoute(
                builder: (context) => SatscardWidget(satscard),
              ),
            );
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
      routes: {
        '/': (context) => Scaffold(
          appBar: AppBar(
            title: const Text('Coinkite Tap Protocol Tester'),
          ),
          body: SingleChildScrollView(
            child: Container(
              padding: const EdgeInsets.all(10),
              child: Column(
                children: [
                  const Text(
                    'Please tap your phone with a Satscard or a Tapsigner',
                    style: textStyle,
                    textAlign: TextAlign.center,
                  )
                ],
              ),
            ),
          ),
        )
      },
    );
  }
}

class SatscardWidget extends StatefulWidget {
  final Satscard satscard;

  const SatscardWidget(this.satscard, {super.key});
  
  @override
  State<SatscardWidget> createState() => SatscardState();
}

class SatscardState extends State<SatscardWidget> {
  late Satscard satscard;

  @override
  void initState() {
    super.initState();
    satscard = widget.satscard;
  }

  @override
  Widget build(BuildContext context) {
    const textStyle = TextStyle(fontSize: 12);
    const spacerSmall = SizedBox(height: 10);
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Satscard Detected'),
        ),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              children: [
                const Text(
                  'CKTapCard',
                  style: TextStyle(fontSize: 20),
                  textAlign: TextAlign.left,
                ),
                spacerSmall,
                Text(
                  'internalHandle: ${satscard.handle.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'internalType: ${satscard.type.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'ident ${satscard.ident}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'appletVersion: ${satscard.appletVersion}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'birthHeight: ${satscard.birthHeight.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'isTestnet: ${satscard.isTestnet.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'authDelay: ${satscard.authDelay.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'isTampered: ${satscard.isTampered.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'isCertsChecked: ${satscard.isCertsChecked.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'needSetup: ${satscard.needSetup.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'isTapsigner: ${satscard.isTapsigner.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                const Text(
                  'Satscard',
                  style: TextStyle(fontSize: 20),
                  textAlign: TextAlign.left,
                ),
                spacerSmall,
                Text(
                  'activeSlot: ${satscard.activeSlot.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'activeSlotIndex: ${satscard.activeSlotIndex.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'numSlots: ${satscard.numSlots.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'hasUnusedSlots: ${satscard.hasUnusedSlots.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
                Text(
                  'isUsedUp: ${satscard.isUsedUp.toString()}',
                  style: textStyle,
                  textAlign: TextAlign.left,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}