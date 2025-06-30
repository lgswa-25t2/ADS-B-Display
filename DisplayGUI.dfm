object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'ADS-B Viewer'
  ClientHeight = 807
  ClientWidth = 1586
  Color = clSnow
  Constraints.MinHeight = 740
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -10
  Font.Name = 'Tahoma'
  Font.Style = [fsBold]
  Menu = MainMenu1
  Position = poScreenCenter
  WindowState = wsMaximized
  OnMouseWheel = FormMouseWheel
  DesignSize = (
    1586
    807)
  TextHeight = 12
  object Label16: TLabel
    Left = 74
    Top = 138
    Width = 22
    Height = 15
    Caption = 'N/A'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Calibri'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label20: TLabel
    Left = 13
    Top = 139
    Width = 28
    Height = 15
    Caption = 'ALT:'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label21: TLabel
    Left = 78
    Top = 138
    Width = 21
    Height = 15
    Caption = 'N/A'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label28: TLabel
    Left = 78
    Top = 246
    Width = 21
    Height = 12
    Caption = 'N/A'
  end
  object Label4: TLabel
    Left = 12
    Top = 247
    Width = 49
    Height = 15
    Caption = 'Engine:'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object RightPanel: TPanel
    Left = 1336
    Top = 0
    Width = 250
    Height = 757
    Align = alRight
    Color = clWindow
    Constraints.MaxWidth = 250
    TabOrder = 0
    object Panel1: TPanel
      Left = -2
      Top = 400
      Width = 248
      Height = 122
      BevelOuter = bvNone
      Color = clSnow
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = [fsBold]
      ParentBackground = False
      ParentFont = False
      TabOrder = 0
      object Label12: TLabel
        Left = 9
        Top = 1
        Width = 53
        Height = 15
        Caption = 'CPA TIME:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object Label19: TLabel
        Left = 9
        Top = 19
        Width = 77
        Height = 15
        Caption = 'CPA DISTANCE:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object CpaTimeValue: TLabel
        Left = 75
        Top = 1
        Width = 31
        Height = 15
        Caption = 'NONE'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object CpaDistanceValue: TLabel
        Left = 103
        Top = 19
        Width = 28
        Height = 15
        Caption = 'None'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object ZoomIn: TPanel
        Left = 6
        Top = 103
        Width = 70
        Height = 18
        Caption = 'Zoom In'
        Color = clMoneyGreen
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 0
        OnClick = ZoomInClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object ZoomOut: TPanel
        Left = 161
        Top = 103
        Width = 70
        Height = 18
        Caption = 'Zoom Out'
        Color = clMoneyGreen
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 1
        OnClick = ZoomOutClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object TimeToGoTrackBar: TTrackBar
        Left = 1
        Top = 59
        Width = 228
        Height = 33
        Max = 1800
        Min = 1
        Frequency = 100
        Position = 1
        TabOrder = 2
        OnChange = TimeToGoTrackBarChange
      end
      object TimeToGoCheckBox: TCheckBox
        Left = 9
        Top = 31
        Width = 105
        Height = 27
        Caption = 'Time-To-Go'
        Checked = True
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        State = cbChecked
        TabOrder = 3
      end
      object TimeToGoText: TStaticText
        Left = 120
        Top = 37
        Width = 76
        Height = 19
        Caption = '00:00:00:000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 4
      end
    end
    object Panel3: TPanel
      Left = 1
      Top = 26
      Width = 248
      Height = 375
      Align = alTop
      BevelEdges = []
      BevelOuter = bvNone
      Color = clSnow
      Constraints.MinHeight = 70
      Ctl3D = True
      ParentBackground = False
      ParentCtl3D = False
      TabOrder = 1
      object Label15: TLabel
        Left = 72
        Top = 235
        Width = 92
        Height = 15
        Caption = 'Areas Of Interest'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object RawStatusLabel: TLabel
        Left = 235
        Top = 103
        Width = 18
        Height = 24
        Caption = '??'
        Font.Charset = ANSI_CHARSET
        Font.Color = clRed
        Font.Height = -20
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object SBSStatusLabel: TLabel
        Left = 235
        Top = 126
        Width = 18
        Height = 24
        Caption = '??'
        Font.Charset = ANSI_CHARSET
        Font.Color = clRed
        Font.Height = -20
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object DrawMap: TCheckBox
        Left = 5
        Top = 8
        Width = 96
        Height = 17
        Caption = 'Display Map'
        Checked = True
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        State = cbChecked
        TabOrder = 0
      end
      object PurgeStale: TCheckBox
        Left = 5
        Top = 31
        Width = 96
        Height = 19
        Caption = 'Purge Stale'
        Checked = True
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        State = cbChecked
        TabOrder = 1
      end
      object CSpinStaleTime: TCSpinEdit
        Left = 107
        Top = 10
        Width = 62
        Height = 21
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -10
        Font.Name = 'Tahoma'
        Font.Style = []
        Increment = 5
        MaxValue = 1000
        MinValue = 5
        ParentFont = False
        TabOrder = 2
        Value = 90
      end
      object AreaListView: TListView
        Left = 5
        Top = 250
        Width = 235
        Height = 97
        Hint = 
          'Area-Selection Guide:'#13#10' - Single Click: Select one area'#13#10'- Ctrl+' +
          'Click: Add/remove areas'#13#10' - ESC: Clear all selections'
        Columns = <
          item
            Caption = 'Area'
            Width = 170
          end
          item
            Caption = 'Color'
            Width = 40
          end>
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        MultiSelect = True
        ReadOnly = True
        RowSelect = True
        ParentFont = False
        TabOrder = 3
        ViewStyle = vsReport
        OnCustomDrawItem = AreaListViewCustomDrawItem
        OnSelectItem = AreaListViewSelectItem
      end
      object Insert: TPanel
        Left = 5
        Top = 350
        Width = 40
        Height = 18
        Caption = 'Insert'
        Color = clMoneyGreen
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 4
        OnClick = InsertClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object Delete: TPanel
        Left = 160
        Top = 350
        Width = 50
        Height = 18
        Caption = 'Delete'
        Color = clCream
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 5
        OnClick = DeleteClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object Complete: TPanel
        Left = 50
        Top = 350
        Width = 60
        Height = 18
        Caption = 'Complete'
        Color = clCream
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 6
        OnClick = CompleteClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object Cancel: TPanel
        Left = 115
        Top = 350
        Width = 41
        Height = 18
        Caption = 'Cancel'
        Color = clCream
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 7
        OnClick = CancelClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object RawConnectButton: TPanel
        Left = 5
        Top = 103
        Width = 104
        Height = 20
        Caption = 'Raw Connect'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 8
        OnClick = RawConnectButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object RawIpAddress: TComboBox
        Left = 115
        Top = 103
        Width = 117
        Height = 23
        DropDownWidth = 117
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 9
        Text = '127.0.0.1'
      end
      object CycleImages: TCheckBox
        Left = 107
        Top = 31
        Width = 96
        Height = 19
        Caption = 'Cycle Images'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 10
        OnClick = CycleImagesClick
      end
      object RawPlaybackButton: TPanel
        Left = 114
        Top = 82
        Width = 117
        Height = 17
        Caption = 'Raw Playback'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 11
        OnClick = RawPlaybackButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object RawRecordButton: TPanel
        Left = 5
        Top = 82
        Width = 104
        Height = 17
        Caption = 'Raw Record'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 12
        OnClick = RawRecordButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object SBSConnectButton: TPanel
        Left = 5
        Top = 126
        Width = 104
        Height = 20
        Caption = 'SBS Connect'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 13
        OnClick = SBSConnectButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object SBSIpAddress: TComboBox
        Left = 115
        Top = 126
        Width = 117
        Height = 23
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 14
        Text = 'data.adsbhub.org'
      end
      object SBSRecordButton: TPanel
        Left = 5
        Top = 150
        Width = 104
        Height = 17
        Caption = 'SBS Record'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 15
        OnClick = SBSRecordButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object SBSPlaybackButton: TPanel
        Left = 114
        Top = 150
        Width = 117
        Height = 17
        Caption = 'SBS Playback'
        Color = clMoneyGreen
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 16
        OnClick = SBSPlaybackButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
      object DisplayAirportCheckBox: TCheckBox
        Left = 5
        Top = 56
        Width = 124
        Height = 17
        Caption = 'Display Airport'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 18
        OnClick = DisplayAirportCheckBoxClick
      end
      object PurgeButton: TPanel
        Left = 175
        Top = 10
        Width = 60
        Height = 22
        Caption = 'Purge'
        Color = clMoneyGreen
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentBackground = False
        ParentFont = False
        TabOrder = 17
        OnClick = PurgeButtonClick
        OnMouseDown = PanelButtonMouseDown
        OnMouseUp = PanelButtonMouseUp
      end
    end
    object Panel2: TPanel
      Left = 1
      Top = 199
      Width = 248
      Height = 59
      Color = clSnow
      ParentBackground = False
      TabOrder = 2
      object MapComboBox: TComboBox
        Left = 5
        Top = 6
        Width = 100
        Height = 23
        Style = csDropDownList
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnCloseUp = MapComboBoxCloseUp
        Items.Strings = (
          'Google Maps'
          'VFR'
          'IFS Low'
          'IFS High'
          'OpenStreetMap')
      end
      object BigQueryCheckBox: TCheckBox
        Left = 5
        Top = 32
        Width = 124
        Height = 17
        Caption = 'BigQuery Upload'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BigQueryCheckBoxClick
      end
      object LiveMapCheckbox: TCheckBox
        Left = 120
        Top = 6
        Width = 110
        Height = 20
        Caption = 'Live Map'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = LiveMapCheckboxClick
      end
      object Outline1: TOutline
        Left = 232
        Top = 48
        Width = 121
        Height = 97
        ItemHeight = 13
        TabOrder = 3
        ItemSeparator = '\'
      end
    end
    object PanelTitle1: TPanel
      Left = 1
      Top = 1
      Width = 248
      Height = 25
      Align = alTop
      BevelOuter = bvNone
      Caption = 'Control Menu '#9650
      Color = clTeal
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindow
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentBackground = False
      ParentFont = False
      TabOrder = 3
      OnClick = PanelTitle1Click
    end
    object Panel7: TPanel
      Left = 1
      Top = 341
      Width = 248
      Height = 415
      Align = alBottom
      Color = clTeal
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = [fsBold]
      ParentBackground = False
      ParentFont = False
      TabOrder = 4
      object PanelTitle2: TLabel
        AlignWithMargins = True
        Left = 4
        Top = 6
        Width = 240
        Height = 15
        Margins.Top = 5
        Align = alTop
        Alignment = taCenter
        Caption = 'Aircraft Information'
        Color = clTeal
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindow
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentColor = False
        ParentFont = False
        Layout = tlCenter
        ExplicitWidth = 110
      end
      object Panel4: TScrollBox
        Left = 1
        Top = 24
        Width = 246
        Height = 390
        HorzScrollBar.Range = 280
        HorzScrollBar.Smooth = True
        VertScrollBar.Range = 797
        VertScrollBar.Smooth = True
        Align = alBottom
        AutoScroll = False
        BevelEdges = []
        BevelInner = bvNone
        BevelOuter = bvNone
        BorderStyle = bsNone
        Color = clSnow
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentColor = False
        ParentFont = False
        TabOrder = 0
        UseWheelForScrolling = True
        object InnePanel: TPanel
          Left = 0
          Top = 0
          Width = 280
          Height = 95
          BevelOuter = bvNone
          Color = clSnow
          Ctl3D = True
          ParentBackground = False
          ParentCtl3D = False
          TabOrder = 0
          object Label1: TLabel
            Left = 5
            Top = 58
            Width = 54
            Height = 15
            Caption = '# Aircraft:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label2: TLabel
            Left = 5
            Top = 21
            Width = 19
            Height = 15
            Caption = 'Lat:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label11: TLabel
            Left = 5
            Top = 76
            Width = 53
            Height = 15
            Caption = 'Viewable:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label3: TLabel
            Left = 5
            Top = 40
            Width = 22
            Height = 15
            Caption = 'Lon:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object AircraftCountLabel: TLabel
            Left = 85
            Top = 58
            Width = 7
            Height = 15
            Caption = '0'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Lon: TLabel
            Left = 39
            Top = 42
            Width = 7
            Height = 15
            Caption = '0'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Lat: TLabel
            Left = 39
            Top = 21
            Width = 7
            Height = 15
            Caption = '0'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object SystemTimeLabel: TLabel
            Left = 4
            Top = 4
            Width = 67
            Height = 15
            Caption = 'System Time'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object ViewableAircraftCountLabel: TLabel
            Left = 85
            Top = 76
            Width = 7
            Height = 15
            Caption = '0'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Shape1: TShape
            Left = 0
            Top = 92
            Width = 280
            Height = 3
            Pen.Color = clSilver
          end
          object SystemTime: TStaticText
            Left = 90
            Top = 4
            Width = 76
            Height = 19
            Caption = '00:00:00:000'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
            TabOrder = 0
          end
        end
        object InnerPannel2: TPanel
          Left = 4
          Top = 97
          Width = 280
          Height = 700
          BevelOuter = bvNone
          Color = clSnow
          ParentBackground = False
          TabOrder = 1
          object SpdLabel: TLabel
            Left = 70
            Top = 76
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object HdgLabel: TLabel
            Left = 70
            Top = 95
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object AltLabel: TLabel
            Left = 70
            Top = 114
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object MsgCntLabel: TLabel
            Left = 70
            Top = 297
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TrkLastUpdateTimeLabel: TLabel
            Left = 125
            Top = 725
            Width = 72
            Height = 15
            Caption = '00:00:00:000'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label14: TLabel
            Left = 5
            Top = 725
            Width = 101
            Height = 15
            Caption = 'LAST UPDATE TIME:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label13: TLabel
            Left = 5
            Top = 297
            Width = 51
            Height = 15
            Caption = 'MSG CNT:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label10: TLabel
            Left = 5
            Top = 115
            Width = 19
            Height = 15
            Caption = 'ALT:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label9: TLabel
            Left = 5
            Top = 96
            Width = 27
            Height = 15
            Caption = 'HDG:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label8: TLabel
            Left = 5
            Top = 77
            Width = 24
            Height = 15
            Caption = 'SPD:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label18: TLabel
            Left = 5
            Top = 19
            Width = 50
            Height = 15
            Caption = 'FLIGHT #:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clNavy
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object FlightNumLabel: TLabel
            Left = 70
            Top = 19
            Width = 22
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clNavy
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object ICAOLabel: TLabel
            Left = 70
            Top = 2
            Width = 47
            Height = 15
            Caption = 'No Hook'
            Font.Charset = ANSI_CHARSET
            Font.Color = clNavy
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object Label5: TLabel
            Left = 5
            Top = 2
            Width = 31
            Height = 15
            Caption = 'ICAO:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clNavy
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object Label22: TLabel
            Left = 5
            Top = 136
            Width = 22
            Height = 15
            Caption = 'S/N:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object SerialNum: TLabel
            Left = 70
            Top = 135
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label23: TLabel
            Left = 4
            Top = 157
            Width = 27
            Height = 15
            Caption = 'MFR:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Manufacturer: TLabel
            Left = 70
            Top = 156
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label24: TLabel
            Left = 5
            Top = 178
            Width = 38
            Height = 15
            Caption = 'Model:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Model: TLabel
            Left = 70
            Top = 177
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label25: TLabel
            Left = 4
            Top = 199
            Width = 53
            Height = 15
            Caption = 'MFR Year:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object MFRYear: TLabel
            Left = 70
            Top = 198
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label26: TLabel
            Left = 4
            Top = 220
            Width = 27
            Height = 15
            Caption = 'Type:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label27: TLabel
            Left = 4
            Top = 241
            Width = 39
            Height = 15
            Caption = 'Engine:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object AirType: TLabel
            Left = 70
            Top = 219
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object EngineType: TLabel
            Left = 70
            Top = 240
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label29: TLabel
            Left = 4
            Top = 261
            Width = 25
            Height = 15
            Caption = 'Cert:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object CeritificatedInfo: TLabel
            Left = 70
            Top = 261
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label30: TLabel
            Left = 4
            Top = 278
            Width = 59
            Height = 15
            Caption = 'Expiration:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object ExpirationData: TLabel
            Left = 70
            Top = 278
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object SubTitleDeparture: TLabel
            Left = 5
            Top = 316
            Width = 142
            Height = 15
            Caption = #9654#9654' Departure Airport '#9664#9664
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object DepartureAirportName: TLabel
            Left = 70
            Top = 334
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label32: TLabel
            Left = 5
            Top = 351
            Width = 30
            Height = 15
            Caption = 'ICAO:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label33: TLabel
            Left = 5
            Top = 334
            Width = 34
            Height = 15
            Caption = 'Name:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DepartureAirportICAO: TLabel
            Left = 70
            Top = 351
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label35: TLabel
            Left = 5
            Top = 367
            Width = 49
            Height = 15
            Caption = 'Location:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DepartureAirportLocation: TLabel
            Left = 70
            Top = 367
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label34: TLabel
            Left = 5
            Top = 383
            Width = 46
            Height = 15
            Caption = 'Country:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label36: TLabel
            Left = 70
            Top = 383
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object subTitleDestination: TLabel
            Left = 5
            Top = 404
            Width = 148
            Height = 15
            Caption = #9654#9654' Destination Airport '#9664#9664
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object Label38: TLabel
            Left = 5
            Top = 425
            Width = 34
            Height = 15
            Caption = 'Name:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DestinationAirportName: TLabel
            Left = 70
            Top = 425
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label39: TLabel
            Left = 5
            Top = 446
            Width = 30
            Height = 15
            Caption = 'ICAO:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DestinationAirportICAO: TLabel
            Left = 70
            Top = 446
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label40: TLabel
            Left = 5
            Top = 467
            Width = 49
            Height = 15
            Caption = 'Location:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DestinationAirportLocation: TLabel
            Left = 72
            Top = 467
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label41: TLabel
            Left = 5
            Top = 488
            Width = 46
            Height = 15
            Caption = 'Country:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object DestinationCounty: TLabel
            Left = 70
            Top = 488
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label43: TLabel
            Left = 5
            Top = 509
            Width = 136
            Height = 15
            Caption = #9654#9654' Transit Airport[1] '#9664#9664
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object Label44: TLabel
            Left = 5
            Top = 530
            Width = 34
            Height = 15
            Caption = 'Name:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport1Name: TLabel
            Left = 70
            Top = 530
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label46: TLabel
            Left = 5
            Top = 551
            Width = 30
            Height = 15
            Caption = 'ICAO:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport1ICAO: TLabel
            Left = 70
            Top = 551
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label48: TLabel
            Left = 5
            Top = 572
            Width = 49
            Height = 15
            Caption = 'Location:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport1Location: TLabel
            Left = 70
            Top = 572
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label50: TLabel
            Left = 5
            Top = 593
            Width = 46
            Height = 15
            Caption = 'Country:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport1Country: TLabel
            Left = 70
            Top = 593
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label45: TLabel
            Left = 3
            Top = 614
            Width = 136
            Height = 15
            Caption = #9654#9654' Transit Airport[2] '#9664#9664
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = [fsBold]
            ParentFont = False
          end
          object Label47: TLabel
            Left = 5
            Top = 635
            Width = 34
            Height = 15
            Caption = 'Name:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport2Name: TLabel
            Left = 72
            Top = 635
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label51: TLabel
            Left = 5
            Top = 656
            Width = 30
            Height = 15
            Caption = 'ICAO:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport2ICAO: TLabel
            Left = 72
            Top = 656
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label53: TLabel
            Left = 5
            Top = 677
            Width = 49
            Height = 15
            Caption = 'Location:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport2Location: TLabel
            Left = 72
            Top = 677
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object Label55: TLabel
            Left = 5
            Top = 698
            Width = 46
            Height = 15
            Caption = 'Country:'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object TransitAirport2Country: TLabel
            Left = 72
            Top = 698
            Width = 20
            Height = 15
            Caption = 'N/A'
            Font.Charset = ANSI_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Calibri'
            Font.Style = []
            ParentFont = False
          end
          object LabelLat: TLabel
            Left = 5
            Top = 40
            Width = 41
            Height = 15
            Caption = 'Lat(AC):'
          end
          object LabelLong: TLabel
            Left = 5
            Top = 59
            Width = 44
            Height = 15
            Caption = 'Lon(AC):'
          end
          object CLatiLabel: TLabel
            Left = 70
            Top = 40
            Width = 20
            Height = 15
            Caption = 'N/A'
          end
          object CLongtLabel: TLabel
            Left = 70
            Top = 59
            Width = 20
            Height = 15
            Caption = 'N/A'
          end
          object Shape2: TShape
            Left = 0
            Top = 130
            Width = 280
            Height = 1
            Pen.Color = clSilver
          end
          object Shape3: TShape
            Left = 0
            Top = 215
            Width = 280
            Height = 1
            Pen.Color = clSilver
          end
          object Shape4: TShape
            Left = 0
            Top = 315
            Width = 280
            Height = 1
            Pen.Color = clSilver
          end
          object Shape5: TShape
            Left = 0
            Top = 507
            Width = 280
            Height = 1
            Pen.Color = clSilver
          end
        end
      end
    end
  end
  object LegendPanel: TPanel
    Left = 0
    Top = 757
    Width = 1586
    Height = 50
    Align = alBottom
    BevelOuter = bvLowered
    Color = clSnow
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -10
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentBackground = False
    ParentFont = False
    TabOrder = 1
    object LegendTitle: TLabel
      Left = 7
      Top = 6
      Width = 117
      Height = 15
      Caption = 'Filter by Aircraft Type'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object HelicopterColorBox: TShape
      Left = 140
      Top = 10
      Width = 12
      Height = 12
      Brush.Color = 26367
      Shape = stCircle
    end
    object HelicopterLabel: TLabel
      Left = 158
      Top = 10
      Width = 57
      Height = 15
      Caption = 'Helicopter'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object MilitaryColorBox: TShape
      Left = 228
      Top = 10
      Width = 12
      Height = 12
      Brush.Color = clLime
      Shape = stCircle
    end
    object MilitaryLabel: TLabel
      Left = 246
      Top = 10
      Width = 45
      Height = 15
      Caption = 'Military'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object CivilianColorBox: TShape
      Left = 301
      Top = 10
      Width = 12
      Height = 12
      Brush.Color = clFuchsia
      Shape = stCircle
    end
    object CivilianLabel: TLabel
      Left = 319
      Top = 10
      Width = 43
      Height = 15
      Caption = 'Civilian'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object UnknownColorBox: TShape
      Left = 374
      Top = 10
      Width = 12
      Height = 12
      Brush.Color = clAqua
      Shape = stCircle
    end
    object UnknownLabel: TLabel
      Left = 392
      Top = 10
      Width = 51
      Height = 15
      Caption = 'Unknown'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object NearAirportColorBox: TShape
      Left = 140
      Top = 31
      Width = 12
      Height = 12
      Brush.Color = clYellow
      Shape = stCircle
    end
    object NearAirportLabel: TLabel
      Left = 158
      Top = 31
      Width = 68
      Height = 15
      Caption = 'Near Airport'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object SelectedColorBox: TShape
      Left = 238
      Top = 31
      Width = 12
      Height = 12
      Brush.Style = bsClear
      Pen.Color = clRed
      Pen.Width = 2
      Shape = stCircle
    end
    object SelectedLabel: TLabel
      Left = 256
      Top = 31
      Width = 45
      Height = 15
      Caption = 'Selected'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object TrackHistoryColorBox: TShape
      Left = 316
      Top = 31
      Width = 12
      Height = 12
      Pen.Color = clGray
    end
    object TrackHistoryLabel: TLabel
      Left = 334
      Top = 31
      Width = 72
      Height = 15
      Caption = 'Track History'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentFont = False
    end
    object AircraftTypeFilterComboBox: TComboBox
      Left = 14
      Top = 27
      Width = 120
      Height = 20
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 0
      Text = 'All'
      OnCloseUp = AircraftTypeFilterComboBoxCloseUp
      Items.Strings = (
        'All'
        'Helicopters'
        'Military'
        'Known Civilian'
        'Unknown')
    end
    object TimeToGoLegendPanel: TPanel
      Left = 469
      Top = 2
      Width = 692
      Height = 48
      BevelEdges = []
      BevelOuter = bvNone
      Color = clSnow
      ParentBackground = False
      TabOrder = 1
      object TimeToGoLegendTitle: TLabel
        Left = 0
        Top = 4
        Width = 210
        Height = 15
        Caption = 'Filter by Altitude:ft (Leader Line color)'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object HighAltitudeShape: TShape
        Left = 224
        Top = 11
        Width = 20
        Height = 5
        Brush.Color = clHotpink
        Pen.Color = clFuchsia
        Pen.Width = 2
      end
      object HighAltitudeLabel: TLabel
        Left = 249
        Top = 6
        Width = 44
        Height = 15
        Caption = '35,000+'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object MedHighAltitudeShape: TShape
        Left = 305
        Top = 11
        Width = 20
        Height = 5
        Brush.Color = clMediumblue
        Pen.Color = clMediumblue
        Pen.Width = 2
      end
      object MedHighAltitudeLabel: TLabel
        Left = 331
        Top = 5
        Width = 80
        Height = 15
        Caption = '25,000-35,000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object MedAltitudeShape: TShape
        Left = 417
        Top = 11
        Width = 20
        Height = 5
        Brush.Color = clAqua
        Pen.Color = clAqua
        Pen.Width = 2
      end
      object MedAltitudeLabel: TLabel
        Left = 447
        Top = 6
        Width = 80
        Height = 15
        Caption = '15,000-25,000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object LowMedAltitudeShape: TShape
        Left = 145
        Top = 32
        Width = 20
        Height = 5
        Brush.Color = clLime
        Pen.Color = clLime
        Pen.Width = 2
      end
      object LowMedAltitudeLabel: TLabel
        Left = 171
        Top = 27
        Width = 73
        Height = 15
        Caption = '8,000-15,000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object LowAltitudeLabel: TLabel
        Left = 281
        Top = 27
        Width = 66
        Height = 15
        Caption = '3,000-8,000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object LowAltitudeShape: TShape
        Left = 255
        Top = 32
        Width = 20
        Height = 5
        Brush.Color = clYellow
        Pen.Color = clYellow
      end
      object VeryLowAltitudeLabel: TLabel
        Left = 390
        Top = 27
        Width = 66
        Height = 15
        Caption = '1,000-3,000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object VeryLowAltitudeShape: TShape
        Left = 364
        Top = 32
        Width = 20
        Height = 5
        Brush.Color = clOrange
        Pen.Color = clOrange
      end
      object LowestAltitudeShape: TShape
        Left = 464
        Top = 32
        Width = 20
        Height = 5
        Brush.Color = clRed
        Pen.Color = clRed
      end
      object LowestAltitudeLabel: TLabel
        Left = 493
        Top = 27
        Width = 35
        Height = 15
        Caption = '1,000-'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = []
        ParentFont = False
      end
      object Label6: TLabel
        Left = 552
        Top = 4
        Width = 82
        Height = 15
        Caption = 'Filter by Speed'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object AltitudeFilterComboBox: TComboBox
        Left = 5
        Top = 25
        Width = 120
        Height = 21
        Style = csDropDownList
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Consolas'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 0
        OnCloseUp = AltitudeFilterComboBoxCloseUp
      end
      object SpeedFilterComboBox: TComboBox
        Left = 556
        Top = 25
        Width = 120
        Height = 21
        Style = csDropDownList
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Consolas'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
        OnCloseUp = SpeedFilterComboBoxCloseUp
      end
    end
  end
  object ObjectDisplay: TOpenGLPanel
    Left = 0
    Top = 0
    Width = 1336
    Height = 757
    Cursor = crCross
    Align = alClient
    TabOrder = 2
    PFDFlags = [f_PFD_DRAW_TO_WINDOW, f_PFD_SUPPORT_OPENGL, f_PFD_DOUBLEBUFFER]
    Font3D_Type.Charset = ANSI_CHARSET
    Font3D_Type.Color = clWindowText
    Font3D_Type.Height = -9
    Font3D_Type.Name = 'Arial'
    Font3D_Type.Style = []
    Font3D_Extrustion = 0.100000001490116100
    Font2D_Enabled = True
    Font2D_Type.Charset = ANSI_CHARSET
    Font2D_Type.Color = clWindowText
    Font2D_Type.Height = -27
    Font2D_Type.Name = 'Arial'
    Font2D_Type.Style = [fsBold]
    OnMouseDown = ObjectDisplayMouseDown
    OnMouseMove = ObjectDisplayMouseMove
    OnMouseUp = ObjectDisplayMouseUp
    OnInit = ObjectDisplayInit
    OnResize = ObjectDisplayResize
    OnPaint = ObjectDisplayPaint
  end
  object PlaybackSpeedPanel: TPanel
    Left = 914
    Top = 0
    Width = 401
    Height = 85
    Anchors = [akTop, akRight]
    Color = clSnow
    ParentBackground = False
    TabOrder = 3
    Visible = False
    DesignSize = (
      401
      85)
    object x5: TLabel
      Left = 194
      Top = 65
      Width = 13
      Height = 12
      Caption = 'x5'
    end
    object x10: TLabel
      Left = 373
      Top = 65
      Width = 20
      Height = 12
      Caption = 'x10'
    end
    object x1: TLabel
      Left = 4
      Top = 65
      Width = 13
      Height = 12
      Caption = 'x1'
    end
    object PlaybackSpeedTrackBar: TTrackBar
      Left = 0
      Top = 32
      Width = 401
      Height = 48
      Anchors = [akTop, akRight]
      ParentShowHint = False
      ShowHint = False
      TabOrder = 0
      OnChange = PlaybackSpeedTrackBarChanged
    end
    object PlaybackControlTitle: TPanel
      Left = 1
      Top = 1
      Width = 399
      Height = 25
      Align = alTop
      Caption = 'Playback Speed Control'
      Color = clTeal
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindow
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = []
      ParentBackground = False
      ParentFont = False
      TabOrder = 1
      OnClick = PanelTitle1Click
    end
  end
  object PlaybackProgressPanel: TPanel
    Left = 1169
    Top = 768
    Width = 417
    Height = 52
    Anchors = [akRight, akBottom]
    Color = clSnow
    ParentBackground = False
    TabOrder = 4
    Visible = False
    object TotalTimeLabel: TLabel
      Left = 352
      Top = 2
      Width = 48
      Height = 12
      Caption = '00:00:00'
    end
    object PlayTimeLabel: TLabel
      Left = 11
      Top = 2
      Width = 48
      Height = 12
      Caption = '00:00:00'
    end
    object PlaybackProgressTrackBar: TTrackBar
      Left = 6
      Top = 14
      Width = 403
      Height = 45
      TabOrder = 0
      OnChange = PlaybackProgressBarChange
    end
  end
  object MainMenu1: TMainMenu
    Left = 24
    object File1: TMenuItem
      Caption = 'File'
      object UseSBSRemote: TMenuItem
        Caption = 'ADS-B Hub'
        OnClick = UseSBSRemoteClick
      end
      object UseSBSLocal: TMenuItem
        Caption = 'ADS-B Local'
        OnClick = UseSBSLocalClick
      end
      object LoadARTCCBoundaries1: TMenuItem
        Caption = 'Load ARTCC Boundaries'
        OnClick = LoadARTCCBoundaries1Click
      end
      object Exit1: TMenuItem
        Caption = 'Exit'
        OnClick = Exit1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object UserManual1: TMenuItem
        Caption = 'User Manual (F1)'
        OnClick = UserManual1Click
      end
      object AboutADSBDisplay1: TMenuItem
        Caption = 'About'
        OnClick = AboutADSBDisplay1Click
      end
    end
  end
  object Timer1: TTimer
    Interval = 500
    OnTimer = Timer1Timer
    Left = 88
  end
  object Timer2: TTimer
    Interval = 5000
    OnTimer = Timer2Timer
    Left = 136
  end
  object IdTCPClientRaw: TIdTCPClient
    OnDisconnected = IdTCPClientRawDisconnected
    OnConnected = IdTCPClientRawConnected
    ConnectTimeout = 0
    Port = 0
    ReadTimeout = -1
    ReuseSocket = rsTrue
    Left = 208
  end
  object RecordRawSaveDialog: TSaveDialog
    DefaultExt = 'raw'
    Filter = 'raw|*.raw'
    Left = 328
  end
  object PlaybackRawDialog: TOpenDialog
    DefaultExt = 'raw'
    Filter = 'raw|*.raw'
    Left = 448
  end
  object IdTCPClientSBS: TIdTCPClient
    OnDisconnected = IdTCPClientSBSDisconnected
    OnConnected = IdTCPClientSBSConnected
    ConnectTimeout = 0
    Port = 0
    ReadTimeout = -1
    ReuseSocket = rsTrue
    Left = 552
  end
  object RecordSBSSaveDialog: TSaveDialog
    DefaultExt = 'sbs'
    Filter = 'sbs|*.sbs'
    Left = 664
  end
  object PlaybackSBSDialog: TOpenDialog
    DefaultExt = 'sbs'
    Filter = 'sbs|*.sbs'
    Left = 784
  end
end
