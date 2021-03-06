/* ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
=========================================================================== */

package GUI;

import data.TimeDiff;
import java.awt.Dimension;
import java.text.NumberFormat;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.Border;

public class StatusBar extends JPanel
{
    private static StatusBar INSTANCE = null;
    public static StatusBar getInstance() { return INSTANCE; }
    
    public static StatusBar make_status_bar( final int width, final int height )
    {
        if ( INSTANCE == null )
            INSTANCE = new StatusBar( width, height );
        return INSTANCE;
    }
    
    private static final String EMPTY_STR = "";
    private static final String ACC_STR = "accession";
    private static final String MAX_LABEL_HINT_DNLD = "total bytes to be downloaded";
    private static final String MAX_LABEL_HINT = "total reads to be processed";
    private static final String PRO_LABEL_HINT_DNLD = "bytes downloaded so far";
    private static final String PRO_LABEL_HINT = "reads processed so far";
    private static final String RPM_LABEL_HINT_DNLD = "bytes per minute";
    private static final String RPM_LABEL_HINT = "reads per minute";
    
    private static final String RPM_FMT_DNLD = "%s bpm";
    private static final String RPM_FMT = "%s rpm";

    private final JLabel acc_label;
    private final JLabel max_label;
    private final JLabel pro_label;
    private final JLabel rpm_label;
    private final JLabel time_label;
    private final JLabel left_label;
    private final NumberFormat nf_inst;
    private boolean is_dnld;
    
    public static void set_is_dnld( final boolean value )
    {
        if ( INSTANCE != null ) INSTANCE.set_dnld( value );
    }
    
    public static void set_acc( final String txt )
    {
        if ( INSTANCE != null ) INSTANCE.set_acc_txt( txt );
    }
    
    public static void set_max( final long value )
    {
        if ( INSTANCE != null ) INSTANCE.set_max_value( value );
    }

    public static void set_pro( final long value )
    {
        if ( INSTANCE != null ) INSTANCE.set_pro_value( value );
    }

    public static void set_time( final long value )
    {
        if ( INSTANCE != null ) INSTANCE.set_time_value( value );
    }

    public static void set_rpm_or_bpm( final long value )
    {
        if ( INSTANCE != null ) INSTANCE.set_rpm_or_bpm_value( value );
    }

    public static void set_left( final String value )
    {
        if ( INSTANCE != null ) INSTANCE.set_left_value( value );
    }

    private void set_dnld( final boolean value )
    {
        is_dnld = value;
        max_label.setToolTipText( is_dnld ? MAX_LABEL_HINT_DNLD : MAX_LABEL_HINT );
        pro_label.setToolTipText( is_dnld ? PRO_LABEL_HINT_DNLD : PRO_LABEL_HINT );
        rpm_label.setToolTipText( is_dnld ? RPM_LABEL_HINT_DNLD : RPM_LABEL_HINT );
    }
    
    private void set_acc_txt( final String txt ) { acc_label.setText( txt ); }
    private void set_max_value( final long value ) { max_label.setText( nf_inst.format( value ) ); }
    private void set_pro_value( final long value ) { pro_label.setText( nf_inst.format( value ) ); }
    private void set_time_value( final long value ) { time_label.setText( TimeDiff.to_str( value ) ); }
    
    private void set_rpm_or_bpm_value( final long value )
    {
        rpm_label.setText( String.format( is_dnld ? RPM_FMT_DNLD : RPM_FMT, 
                                          NumberFormat.getInstance().format( value ) ) );
    }
    
    private void set_left_value( final String txt ) { left_label.setText( txt ); }
    
    private void set_dim( final JComponent c, final Dimension dim )
    {
        c.setMinimumSize( dim );
        c.setPreferredSize( dim );
        c.setMaximumSize( dim );
    }
    
    private Border make_outer_border()
    {
        Border one = BorderFactory.createLoweredBevelBorder();
        Border two = BorderFactory.createEmptyBorder( 2, 2, 2, 2 );
        return BorderFactory.createCompoundBorder( one, two );
    }

    private Border make_label_border()
    {
        Border one = BorderFactory.createRaisedBevelBorder();
        Border two = BorderFactory.createEmptyBorder( 2, 2, 2, 2 );
        return BorderFactory.createCompoundBorder( one, two );
    }
    
    private JLabel make_label( final int width, final int height,
            final String s, final String hint )
    {
        JLabel res = new JLabel( s );
        res.setBorder( make_label_border() );
        set_dim( res, new Dimension( width, height ) );
        res.setHorizontalAlignment( SwingConstants.CENTER );
        res.setToolTipText( hint );
        return res;
    }
    
    private JPanel make_left_box()
    {
        JPanel res = new JPanel();
        res.setLayout( new BoxLayout( res, BoxLayout.LINE_AXIS ) );
        res.add( acc_label );
        Dimension dim = new Dimension( acc_label.getPreferredSize() );
        dim.width += 2;
        set_dim( res, dim );
        return res;
    }

    private JPanel make_top_box( final int label_h )
    {
        JPanel res = new JPanel();
        res.setLayout( new BoxLayout( res, BoxLayout.LINE_AXIS ) );
        set_dim( res, new Dimension( Short.MAX_VALUE, label_h ) );
        res.add( pro_label );
        res.add( make_label( 30, label_h, "of", EMPTY_STR ) );
        res.add( max_label );
        res.add( make_label( 50, label_h, " --- ", EMPTY_STR ) );
        res.add( rpm_label );
        return res;
    }
    
    private JPanel make_bottom_box( final int label_h )
    {
        JPanel res = new JPanel();
        res.setLayout( new BoxLayout( res, BoxLayout.LINE_AXIS ) );
        set_dim( res, new Dimension( Short.MAX_VALUE, label_h ) );        
        res.add( time_label );
        res.add( left_label );
        return res;
    }

    private JPanel make_right_box( final int label_h )
    {
        JPanel res = new JPanel();
        res.setLayout( new BoxLayout( res, BoxLayout.Y_AXIS ) );
        res.add( make_top_box( label_h ) );
        res.add( make_bottom_box( label_h ) );        
        return res;
    }
    
    private StatusBar( final int width, final int height )
    {
        super();
        nf_inst = NumberFormat.getInstance();
        
        this.setBorder( make_outer_border() );
        this.setPreferredSize( new Dimension( width, height ) );
        this.setLayout( new BoxLayout( this, BoxLayout.X_AXIS ) );

        // make the labels...
        int label_h = ( height / 2 ) - 4;
        acc_label = make_label( 100, height - 4, EMPTY_STR, ACC_STR );
        pro_label = make_label( 120, label_h, EMPTY_STR, PRO_LABEL_HINT );
        max_label = make_label( 120, label_h, EMPTY_STR, MAX_LABEL_HINT );
        rpm_label = make_label( 120, label_h, EMPTY_STR, RPM_LABEL_HINT );
        time_label = make_label( 220, label_h, EMPTY_STR, "elapsed time" );
        left_label = make_label( 220, label_h, EMPTY_STR, "time left" );

        is_dnld = false;
        add( make_left_box() );
        add( make_right_box( label_h ) );
    }
}
