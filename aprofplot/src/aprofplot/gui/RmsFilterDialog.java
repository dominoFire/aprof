package aprofplot.gui;

public class RmsFilterDialog extends javax.swing.JDialog {

    /**
     * Creates new form RmsFilterDialog
     */
    public RmsFilterDialog(java.awt.Frame parent, boolean modal, String[] criteria) {

        super(parent, modal);
        initComponents();

        setSize(240, 300);
        setLocationRelativeTo(parent);

        if (criteria == null) {
            return;
        }

        if (criteria[0] != null) {
            jCheckBox1.setSelected(true);
            jTextField1.setEnabled(true);
            jTextField1.setText(criteria[0]);
        }
        if (criteria[1] != null) {
            jCheckBox2.setSelected(true);
            jTextField2.setEnabled(true);
            jTextField2.setText(criteria[1]);
        }
        if (criteria[2] != null) {
            jCheckBox3.setSelected(true);
            jTextField3.setEnabled(true);
            jTextField3.setText(criteria[2]);
        }

        if (((MainWindow) this.getParent()).isInputMetricRms()) {
            jCheckBox2.setText("#RMS");
        } else {
            jCheckBox2.setText("#DRMS");
        }
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        jCheckBox2 = new javax.swing.JCheckBox();
        jTextField2 = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        jPanel3 = new javax.swing.JPanel();
        jCheckBox1 = new javax.swing.JCheckBox();
        jTextField1 = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        jPanel5 = new javax.swing.JPanel();
        jCheckBox3 = new javax.swing.JCheckBox();
        jTextField3 = new javax.swing.JTextField();
        jPanel2 = new javax.swing.JPanel();
        jButton1 = new javax.swing.JButton();
        jButton2 = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Filter sms");
        setIconImage((new javax.swing.ImageIcon(getClass().getResource("/aprofplot/gui/resources/Filter-icon.png"))).getImage());
        setResizable(false);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Criteria"));
        jPanel1.setLayout(new java.awt.GridLayout(3, 1));

        jPanel4.setLayout(new javax.swing.BoxLayout(jPanel4, javax.swing.BoxLayout.LINE_AXIS));

        jCheckBox2.setText("Rms >= ");
        jCheckBox2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jCheckBox2ActionPerformed(evt);
            }
        });
        jPanel4.add(jCheckBox2);

        jTextField2.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jTextField2.setEnabled(false);
        jTextField2.setMaximumSize(new java.awt.Dimension(100, 27));
        jTextField2.setMinimumSize(new java.awt.Dimension(100, 27));
        jTextField2.setPreferredSize(new java.awt.Dimension(100, 27));
        jPanel4.add(jTextField2);
        jPanel4.add(jLabel2);

        jPanel1.add(jPanel4);

        jPanel3.setLayout(new javax.swing.BoxLayout(jPanel3, javax.swing.BoxLayout.LINE_AXIS));

        jCheckBox1.setText("MaxCost >= ");
        jCheckBox1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jCheckBox1ActionPerformed(evt);
            }
        });
        jPanel3.add(jCheckBox1);

        jTextField1.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jTextField1.setEnabled(false);
        jTextField1.setMaximumSize(new java.awt.Dimension(100, 27));
        jTextField1.setMinimumSize(new java.awt.Dimension(100, 27));
        jTextField1.setPreferredSize(new java.awt.Dimension(100, 27));
        jPanel3.add(jTextField1);
        jPanel3.add(jLabel1);

        jPanel1.add(jPanel3);

        jPanel5.setLayout(new javax.swing.BoxLayout(jPanel5, javax.swing.BoxLayout.LINE_AXIS));

        jCheckBox3.setText("Freq >=  ");
        jCheckBox3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jCheckBox3ActionPerformed(evt);
            }
        });
        jPanel5.add(jCheckBox3);

        jTextField3.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jTextField3.setEnabled(false);
        jTextField3.setMaximumSize(new java.awt.Dimension(100, 27));
        jTextField3.setMinimumSize(new java.awt.Dimension(100, 27));
        jTextField3.setPreferredSize(new java.awt.Dimension(100, 27));
        jPanel5.add(jTextField3);

        jPanel1.add(jPanel5);

        getContentPane().add(jPanel1, java.awt.BorderLayout.CENTER);

        jPanel2.setBorder(javax.swing.BorderFactory.createEmptyBorder(1, 3, 3, 3));
        jPanel2.setLayout(new javax.swing.BoxLayout(jPanel2, javax.swing.BoxLayout.LINE_AXIS));

        jButton1.setText("    Ok    ");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton1ActionPerformed(evt);
            }
        });
        jPanel2.add(javax.swing.Box.createHorizontalGlue());
        jPanel2.add(jButton1);

        jButton2.setText(" Cancel ");
        jButton2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton2ActionPerformed(evt);
            }
        });
        jPanel2.add(jButton2);

        getContentPane().add(jPanel2, java.awt.BorderLayout.SOUTH);

        pack();
    }// </editor-fold>//GEN-END:initComponents

	private void jCheckBox1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jCheckBox1ActionPerformed

        // max cost checkbox
        if (jCheckBox1.isSelected()) {
            jTextField1.setEnabled(true);
        } else {
            jTextField1.setEnabled(false);
        }

	}//GEN-LAST:event_jCheckBox1ActionPerformed

	private void jCheckBox2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jCheckBox2ActionPerformed

        // rms checkbox
        if (jCheckBox2.isSelected()) {
            jTextField2.setEnabled(true);
        } else {
            jTextField2.setEnabled(false);
        }

	}//GEN-LAST:event_jCheckBox2ActionPerformed

	private void jCheckBox3ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jCheckBox3ActionPerformed

        // frequency checkbox
        if (jCheckBox3.isSelected()) {
            jTextField3.setEnabled(true);
        } else {
            jTextField3.setEnabled(false);
        }

	}//GEN-LAST:event_jCheckBox3ActionPerformed

	private void jButton2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton2ActionPerformed

        // Pressed CANCEL button
        this.dispose();

	}//GEN-LAST:event_jButton2ActionPerformed

	private void jButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton1ActionPerformed

        // Pressed OK button
        if (validateUserInput()) {

            this.dispose();
            String[] criteria = new String[3];
            if (jCheckBox1.isSelected()) {
                criteria[0] = jTextField1.getText();
            }
            if (jCheckBox2.isSelected()) {
                criteria[1] = jTextField2.getText();
            }
            if (jCheckBox3.isSelected()) {
                criteria[2] = jTextField3.getText();
            }
            ((MainWindow) this.getParent()).setRmsTableFilter(criteria);
        }
	}//GEN-LAST:event_jButton1ActionPerformed

    private boolean validateUserInput() {

        if (jCheckBox1.isSelected()) {
            try {

                double time = Double.parseDouble(jTextField1.getText());
                if (time < 0 || Math.ceil(time) != time) {
                    throw (new Exception());
                }

            } catch (Exception e) {
                javax.swing.JOptionPane.showMessageDialog(this,
                    "Time must be a positive integer",
                    "Invalid input",
                    javax.swing.JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }
        if (jCheckBox2.isSelected()) {
            try {

                int rms = Integer.parseInt(jTextField2.getText());
                if (rms < 0) {
                    throw (new Exception());
                }

            } catch (Exception e) {
                javax.swing.JOptionPane.showMessageDialog(this,
                    "Rms must a positive integer",
                    "Invalid input",
                    javax.swing.JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }
        if (jCheckBox3.isSelected()) {

            try {
                double freq = Double.parseDouble(jTextField3.getText());
                if (freq < 0 || Math.ceil(freq) != freq) {
                    throw (new Exception());
                }
            } catch (Exception e) {
                javax.swing.JOptionPane.showMessageDialog(this,
                    "Freq must be a positive integer",
                    "Invalid input",
                    javax.swing.JOptionPane.ERROR_MESSAGE);
                return false;
            }

        }
        return true;
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton2;
    private javax.swing.JCheckBox jCheckBox1;
    private javax.swing.JCheckBox jCheckBox2;
    private javax.swing.JCheckBox jCheckBox3;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JTextField jTextField1;
    private javax.swing.JTextField jTextField2;
    private javax.swing.JTextField jTextField3;
    // End of variables declaration//GEN-END:variables

}
